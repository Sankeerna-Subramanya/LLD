                Application threads
                 /      |      \
                /       |       \
               ▼        ▼        ▼
           logger()  logger()  logger()
                \       |       /
                 \      |      /
                  ▼     ▼     ▼
              ┌───────────────┐
              │     Queue     │
              └───────┬───────┘
                      │
                 mutex + CV
                      │
                      ▼
              ┌───────────────┐
              │ Logger Thread │
              └───────┬───────┘
                      │
                 slow disk I/O
                      │
                      ▼
                  server.log
