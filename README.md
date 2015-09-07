# Clax

## Command-Line Usage

    usage: clax [-n] [-l log_file] [-e entropy_file]

    Options:

       common
       ------
       -r <root>          home directory
       -l <log_file>      path to log file

       ssl
       ---
       -n                 do not use ssl
       -e <entropy_file>  path to entropy file
       -t <cert_file>     path to cert file (CA included)
       -p <key_file>      path to private key file

## API Documentation

### Overview

### Authentication

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
