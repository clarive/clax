# Clax Installation

Clax needs an Inetd daemon running (`inetd`/`xinetd` on Unix or `wininetd` on Windows).

## Windows

Windows package already has `wininetd.exe` file in it and sample configuration files.

1. Copy `wininetd.conf` to `C:\Windows\` and adjust the paths accordingly

    ```
    11801 none C:\Users\clarive\clax.exe -c C:/Users/clarive/clax.ini
    ```

2. Install `wininetd.exe` service. This can be done using a `wininetd-install.bat` file (run as administrator).

    ```
    wininetd.exe --install
    ```

3. Make sure the service is running in Services Control Panel
4. Configure `clax.ini`
5. Check that `http://localhost:11801` is returning "Hello world" message

## Linux

### inetd

1. Make sure `/etc/services` has `clax` service

    ```
    clax 11801/tcp
    ```

2. Add the following line to `/etc/inetd.conf`

    ```
    clax     stream tcp nowait <user> <path-to>/clax clax -c <path-to>/clax.ini
    ```

3. Restart `inetd`
4. Check that `http://localhost:11801` is returning "Hello world" message

### xinetd

1. Make sure `/etc/services` has `clax` service

    ```
    clax 11801/tcp
    ```

2. Create a file named `clax-stream` in `/etc/xinetd.d/` with:

    ```
    service clax
    {
        flags           = REUSE
        socket_type     = stream
        wait            = no
        user            = <user>
        server          = <path-to>/clax
        server_args     = -c <path-to>/clax.ini
        log_on_failure  += USERID
    }
    ```

3. Restart `xinetd`
4. Check that `http://localhost:11801` is returning "Hello world" message
