#include <postgresql/libpq-fe.h>

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

void setup(const char *dsn, int items)
{
  auto connection = PQconnectdb(dsn);
  assert(PQstatus(connection) == CONNECTION_OK);
  auto result = PQexec(connection, "DROP TABLE IF EXISTS queue");
  assert(PQresultStatus(result) == PGRES_COMMAND_OK);
  result = PQexec(connection, "CREATE TABLE queue (id SERIAL PRIMARY KEY, data TEXT NOT NULL, status TEXT NOT NULL)");
  assert(PQresultStatus(result) == PGRES_COMMAND_OK);
  std::string statement = "INSERT INTO queue SELECT generate_series(1, " + std::to_string(items) + "), 'foo', 'NEW'";
  result = PQexec(connection, statement.c_str());
  assert(PQresultStatus(result) == PGRES_COMMAND_OK);
}

void work(const char *dsn, bool skip)
{
  auto connection = PQconnectdb(dsn);
  assert(PQstatus(connection) == CONNECTION_OK);
  for (;;) {
    auto result = PQexec(connection, "BEGIN");
    assert(PQresultStatus(result) == PGRES_COMMAND_OK);
    PQclear(result);
    result = PQexec(connection,
                    skip ? "SELECT id, data FROM queue WHERE status = 'NEW' LIMIT 1 FOR UPDATE SKIP LOCKED"
                         : "SELECT id, data FROM queue WHERE status = 'NEW' LIMIT 1 FOR UPDATE");
    assert(PQresultStatus(result) == PGRES_TUPLES_OK);
    if (PQntuples(result) == 1) {
      assert(PQnfields(result) == 2);
      std::string id = PQgetvalue(result, 0, 0);
      std::string data = PQgetvalue(result, 0, 1);
      PQclear(result);
      std::string update = "UPDATE queue SET status = 'WORK' WHERE id = '";
      update += id;
      update += "'";      
      result = PQexec(connection, update.c_str());
      assert(PQresultStatus(result) == PGRES_COMMAND_OK);
      result = PQexec(connection, "COMMIT");
      assert(PQresultStatus(result) == PGRES_COMMAND_OK);    
      result = PQexec(connection, "BEGIN");
      assert(PQresultStatus(result) == PGRES_COMMAND_OK);
      update = "DELETE FROM queue WHERE id = '";
      update += id;
      update += "'";
      result = PQexec(connection, update.c_str());
      assert(PQresultStatus(result) == PGRES_COMMAND_OK);
    } else {
      PQclear(result);
      break;
    }
    result = PQexec(connection, "COMMIT");
    assert(PQresultStatus(result) == PGRES_COMMAND_OK);    
    PQclear(result);
  }
  PQfinish(connection);
}

int main(int argc, char *argv[])
{
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0] << " <connstr> <items> <threads> <skip(0/1)>\n";
    return 1;
  }
  
  auto dsn = argv[1];
  auto items = std::atoi(argv[2]);
  auto thread_count = std::atoi(argv[3]);
  bool skip = std::atoi(argv[4]);

  setup(dsn, items);

  std::vector<std::thread> threads;
  for (auto i = 0; i < thread_count; ++i)
    threads.push_back(std::thread([&](){ work(dsn, skip); }));
  for(auto& thread: threads)
    thread.join();
  
  return 0;
}
