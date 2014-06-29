A test program, and hopefully in future a suite of tests, for 
experimenting with SKIP LOCKED.

http://www.postgresql.org/message-id/CADLWmXUkZq949q3YfHuyhV+90MnQ7LkkavtOv9MMbvqjzKOmQw@mail.gmail.com

As a simple example for people wondering what the point of this
feature is, I created a table work (id, data, status)
and then create 10,000 items with status 'NEW' and then started
a number of worker threads that did the following pair of
transactions, with and without SKIP LOCKED on the end of the
SELECT statement, until all rows were deleted:

    BEGIN
    SELECT id, data FROM work WHERE status = 'NEW' LIMIT 1 FOR UPDATE
    -- if no rows returned, then finish
    UPDATE work SET status = 'WORK' WHERE id = $id
    COMMIT
    
    BEGIN
    DELETE FROM work WHERE id = $id
    COMMIT

Here are the times taken to process all items, in elapsed seconds, on
a slow laptop (i5 4 core with an SSD, with default postgresql.conf
except for checkpoint_segments set to 300):

    | Threads | default |  SKIP |
    |       1 |   26.78 | 27.02 |
    |       2 |   23.46 | 22.00 |
    |       3 |   22.02 | 14.83 |
    |       4 |   22.59 | 11.16 |
    |       5 |   22.37 |  9.05 |
    |       6 |   22.55 |  7.66 |
    |       7 |   22.46 |  6.69 |
    |       8 |   22.57 |  8.39 |
    |       9 |   22.40 |  8.38 |
    |      10 |   22.38 |  7.93 |
    |      11 |   22.43 |  6.86 |
    |      12 |   22.34 |  6.77 |

