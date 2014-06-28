CONNSTR="dbname=postgres port=5433 host=127.0.0.1"
ITEMS=1000

for SKIP in 0 1 ; do
  for THREADS in 1 2 3 4 5 6 7 8 9 10 11 12 ; do
    TIME=` /usr/bin/time --format=%e ./test "$CONNSTR" $ITEMS $THREADS $SKIP 2>&1 `
    echo "$SKIP $THREADS $TIME"
  done
done
