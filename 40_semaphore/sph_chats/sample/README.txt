Compile and run server:
$ gcc spooler.c -o spooler
$ >/tmp/shared_memory_key
$ >/tmp/sem-mutex-key
$ >/tmp/sem-buffer-count-key
$ >/tmp/sem-spool-signal-key
$ ./spooler
