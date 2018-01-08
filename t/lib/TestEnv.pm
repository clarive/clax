package TestEnv;

use strict;
use warnings;

use MIME::Base64 ();
use HTTP::Tiny;

sub build_ua {
    my $class = shift;

    HTTP::Tiny->new(
        default_headers => {
            Authorization => 'Basic '
              . MIME::Base64::encode_base64( 'username:password', '' )
        },
        @_
    );
}

sub endpoint { $ENV{TEST_CLAX_ENDPOINT} // 'http://localhost:11801' }

1;
