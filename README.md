# Clax

Clax is a portable HTTP(s) remote deployment agent that can run commands, exchange files and more. Clax can read
requests from stdin and write responses to stdout, which makes it suitable for inetd integration.

[![Build Status](https://travis-ci.org/clarive/clax.svg?branch=master)](https://travis-ci.org/clarive/clax)

## Tested Platforms

- Debian GNU/Linux x86_64
- FreeBSD 10.3
- Mac OS 10.9 Mavericks
- Cygwin x86_64
- Windows 2003, 2008, 2012
- Solaris 10 i86pc
- z/OS 390
- Raspbian ARMv7

## Compiling, testing and installing

Clax is statically linked, so it can be moved around without worring about the libraries. Compiling is as simple as:

    $ make

Clax has a rich unit and functional test suites.

    $ make check-all

# Command-Line Usage

    usage: clax [options]

       -l <log_file>     path to log file (REQUIRED)
       -c <config_file>  path to configuration file (defaults to clax.ini
                             in binary location directory)

# Configuration

Configuration file is an `INI` file with the following content:

```
[bind]
host = 0.0.0.0
port = 11801

[http_basic_auth]
username = username
password = password

[ssl]
enabled = yes
verify = yes
cert_file = ssl/server.pem
key_file = ssl/server.key
```

## Windows Service

Clax can be run as a windows service. Installation and control can be done via `sc` command:

    # Install Clax service (make sure there a <space> after `=`)
    sc create clax binPath= "C:\clax.exe -l C:\clax.log -c C:\clax.ini" start= auto

    # Start the service
    sc start clax

    # Query the service status
    sc query clax

    # Stop the service
    sc stop clax

# REST API Documentation

## Overview

## Authorization

Authentication is done via basic authentication and/or SSL certificates (client verification).

### Basic authorization

Standard basic authorization described in [rfc2617](http://tools.ietf.org/html/rfc2617).

#### Example

    curl http://username:password@clax.local:11801

### Generating SSL certificates

    # Generate CA certificate
    openssl req -out ca.pem -new -x509 -subj '/CN=company'

    # Create serial file
    echo -n '00' > file.srl

    # Generate server certificate
    openssl genrsa -out server.key 2048

    # Sign server certificate
    openssl req -key server.key -new -out server.req
    openssl x509 -req -in server.req -CA ca.pem -CAkey privkey.pem -CAserial file.srl -out server.pem -subj '/CN=clax-server'

    # Generate client certificate
    openssl genrsa -out client.key 2048 -subj '/CN=client'

    # Sign client certificate
    openssl req -key client.key -new -out client.req
    openssl x509 -req -in client.req -CA ca.pem -CAkey privkey.pem -CAserial file.srl -out client.pem

    # Convert client certificate to PKCS12
    openssl pkcs12 -export -in client.pem -inkey client.key -out client.p12

#### Example (with client SSL verification)

    # If you don't want to validate server certificate (without client authentication, option `-k` in clax)
    curl -k https://clax.local/

    # If you don't want to validate server certificate (with client authentication)
    curl -k --cert ssl/client.pem --key ssl/client.key https://clax.local/

    # If you want to validate server certificate
    curl --cacert ssl/ca.pem --cert ssl/client.pem --key ssl/client.key https://clax.local/

    # or if you want to use PKCS12 certificate
    curl --cacert ssl/ca.pem -E ssl/client.p12 https://clax.local/

## Client Errors

There are several types of errors.

1. Client sent invalid HTTP request

        HTTP/1.1 400 Bad Request
        Content-Length: *

        {"message":"Bad request"}

1. Client sent invalid parameters

        HTTP/1.1 422 Unprocessible Entity
        Content-Length: *

        {"message":"Invalid params"}

## Server Errors

1. An error occured on the server-side

        HTTP/1.1 500 System Error
        Content-Length: *

        {"message":"Cannot save file"}

## File Management

### Check if file exists

    HEAD /tree/filename
    HEAD /tree/some/sub/directory/filename
    HEAD /tree//absolute/path
    HEAD /tree/C:/absolute/path

Same as downloading a file without actually receiving the body.

#### Response

    HTTP/1.1 200 OK
    Content-Type: application/octet-stream
    Pragma: no-cache
    Content-Length: 13915
    Last-Modified: Thu, 10 Sep 2015 14:32:19 GMT
    Content-Length: 123456

#### Example

    curl -I http://clax-server/tree/my-file

### Download file

    GET /tree/filename
    GET /tree/some/sub/directory/filename

Download a file.

#### Response

    HTTP/1.1 200 OK
    Content-Type: application/octet-stream
    Pragma: no-cache
    Last-Modified: Thu, 10 Sep 2015 14:32:19 GMT
    Transfer-Encoding: chunked

    ...<file content>...

#### Example

    curl http://clax-server/tree/my-file

### Upload file

    POST /tree/
    POST /tree/some/sub/directory

Upload file to the server.

#### URL Params

Required

None

Optional

* `name=[string]`

    Save file as a different name

* `crc32=[hex string]`

    CRC32 to calculate

* `time=[int]`

    Set atime/mtime to the provided value

#### Data

File data in bytes.

#### Response

    204 OK

#### Example

    curl --data-binary @path_to_file http://clax.local:11801/tree/

### Delete path

    DELETE /tree/path/to/file

Delete a file.

#### URL Params

Required

None

#### Response

    204 OK

#### Example

    curl -X DELETE http://clax-server/tree/path/to/file

## Run Command

    POST /command

Runs a command and returns the chunks of the output. The special trailing headers provide information when command
finishes. `X-Clax-PID` holds the process `PID`, `X-Clax-Exit` holds the exit code and `X-Clax-Status` holds the
execution status, which can be `success`, `error` or `timeout`.

#### Data Params

Required

* `command=[string]`

Optional

* `timeout=[integer]`

#### Response

    HTTP/1.1 200 OK
    Transfer-Encoding: chunked
    Trailer: X-Clax-Exit, X-Clax-Status
    X-Clax-PID: 10566

    4
    foo

    0
    X-Clax-Exit: 0
    X-Clax-Status: success

#### Example

    curl -d 'command=echo foo' http://clax-server/command

## Contributors

In order of appearance:

* [ktecho](https://github.com/ktecho)

## Copyright

Clax is a portable HTTP(s) remote deployment agent
Copyright (C) 2015  Clarive Software

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
