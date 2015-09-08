# Clax

Clax is a portable HTTP(s) remote deployment agent that can run commands, exchange files and more. Clax can read
requests from stdin and write responses to stdout, which makes it suitable for inetd integration.

## Command-Line Usage

    usage: clax [-n] [-l log_file] [-e entropy_file]

    Options:

       common
       ------
       -r <root>          home directory (required, will chdir to it)
       -l <log_file>      path to log file (default: stderr)

       ssl
       ---
       -n                 do not use ssl at all (default: on)
       -k                 do not verify client certificate (default: on)
       -t <cert_file>     path to cert file (required if ssl, CA included)
       -p <key_file>      path to private key file (required if ssl)
       -e <entropy_file>  path to entropy file (needed on some systems)

## API Documentation

### Overview

### Authentication

Authentication is done via SSL certificates (client authentication).

#### Generating SSL certificates

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

#### Example (curl with client SSL certificate)

    # If you don't want to validate server certificate (without client authentication, option `-k` in clax)
    curl -k https://clax.local/

    # If you don't want to validate server certificate (with client authentication)
    curl -k --cert ssl/client.pem --key ssl/client.key https://clax.local/

    # If you want to validate server certificate
    curl --cacert ssl/ca.pem --cert ssl/client.pem --key ssl/client.key https://clax.local/

    # or if you want to use PKCS12 certificate
    curl --cacert ssl/ca.pem -E ssl/client.p12 https://clax.local/

### Upload File

Upload file to the server via multipart form. The file is saved to the clax home directory. See **URL Params** on how to
modify this behaviour.

#### URL

    /upload

#### Method

    POST

#### URL Params

Required

None

Optional

* `file=[string]`

    Save file as a different name

* `dir=[string]`

    Save file to the different directory

* `crc32=[hex string]` 

    CRC32 to calculate

#### Data Params

Required

* `file=[bytes]`

#### Successful Response

* Code:

    200

* Content:

    {"status":"ok"}

#### Error Response:

* Code:

    400

* Error:

    Bad request

* Reason:

    Bad params were passed.

OR

* Code:

    500

* Error:

    System error

* Reason:

    Some system error, like file cannot be opened.

#### Example

    curl -F 'file=@path_to_file' http://clax-server/upload

#### Notes

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
