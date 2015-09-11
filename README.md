# Clax

Clax is a portable HTTP(s) remote deployment agent that can run commands, exchange files and more. Clax can read
requests from stdin and write responses to stdout, which makes it suitable for inetd integration.

# Command-Line Usage

    usage: clax [options]

    Options:

       common
       ------
       -c <config_file>        path to configuration file
       -z                      print default configuration
       -r <root>               home directory (required, will chdir to it)
       -l <log_file>           path to log file (default: stderr)
       -a <username:password>  basic authentication credentials

       ssl
       ---
       -n                      do not use ssl at all (default: on)
       -k                      do not verify client certificate (default: on)
       -t <cert_file>          path to cert file (required if ssl, CA included)
       -p <key_file>           path to private key file (required if ssl)
       -e <entropy_file>       path to entropy file (needed on some systems)

# API Documentation

## Overview

## Authorization

Authentication is done via basic authentication and/or SSL certificates (client verification).

### Basic authorization

Standard basic authorization described in [rfc2617](http://tools.ietf.org/html/rfc2617).

#### Example

    curl http://clax.local/ -u 'clax:password'

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

    HEAD /tree/:filename
    HEAD /tree/:some/:sub/:directory/:filename

Same as downloading a file without actually receiving the body.

#### Response

    HTTP/1.1 200 OK
    Content-Type: application/octet-stream
    Content-Disposition: attachment; filename="filename"
    Pragma: no-cache
    Content-Length: 13915
    Last-Modified: Thu, 10 Sep 2015 14:32:19 GMT

#### Example

    curl -X HEAD http://clax-server/tree/my-file

### Download file

    GET /tree/:filename
    GET /tree/:some/:sub/:directory/:filename

Download a file as attachment.

#### Response

    HTTP/1.1 200 OK
    Content-Type: application/octet-stream
    Content-Disposition: attachment; filename="filename"
    Pragma: no-cache
    Content-Length: 13915
    Last-Modified: Thu, 10 Sep 2015 14:32:19 GMT

    ...<file content>...

#### Example

    curl http://clax-server/tree/my-file

### Upload file

    POST /tree/
    POST /tree/:some/:sub/:directory

Upload file to the server via multipart form.

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

#### Data Params

Required

* `file=[bytes]`

#### Response

    200 OK

    {"status":"ok"}

#### Example

    curl -F 'file=@path_to_file' http://clax-server/tree/

## Run Command

    POST /command

Runs a command returns the chunks of the output. `X-Clax-PID` holds the process `PID` and `X-Clax-Exit` holds the exit
code.

#### Data Params

Required

* `command=[string]`

Optional

* `timeout=[integer]`

#### Response

    HTTP/1.1 200 OK
    Transfer-Encoding: chunked
    Trailer: X-Clax-Exit
    X-Clax-PID: 10566

    4
    foo

    0
    X-Clax-Exit: 0

#### Example

    curl -d 'command=echo foo' http://clax-server/command

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
