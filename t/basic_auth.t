use strict;
use warnings;
use lib 't/lib';

use Test::More;
use TestEnv;

use MIME::Base64 ();

subtest 'success' => sub {
    my $response = TestEnv->build_ua->get( TestEnv->endpoint );

    ok $response->{success};
};

subtest 'missing header' => sub {
    my $response =
      TestEnv->build_ua( default_headers => {} )->get( TestEnv->endpoint );

    ok !$response->{success};
    is $response->{status}, 401;
};

subtest 'invalid header' => sub {
    my $response =
      TestEnv->build_ua( default_headers => { Authorization => 'foobar' } )
      ->get( TestEnv->endpoint );

    ok !$response->{success};
    is $response->{status}, 401;
};

subtest 'invalid base64 in header' => sub {
    my $response =
      TestEnv->build_ua(
        default_headers => { Authorization => 'Basic foobar' } )
      ->get( TestEnv->endpoint );

    ok !$response->{success};
    is $response->{status}, 401;
};

subtest 'no colon' => sub {
    my $response = TestEnv->build_ua(
        default_headers => {
            Authorization => 'Basic '
              . MIME::Base64::encode_base64( 'username', '' )
        }
    )->get( TestEnv->endpoint );

    ok !$response->{success};
    is $response->{status}, 401;
};

subtest 'wrong username' => sub {
    my $response = TestEnv->build_ua(
        default_headers => {
            Authorization => 'Basic '
              . MIME::Base64::encode_base64( 'wrong:password', '' )
        }
    )->get( TestEnv->endpoint );

    ok !$response->{success};
    is $response->{status}, 401;
};

subtest 'wrong password' => sub {
    my $response = TestEnv->build_ua(
        default_headers => {
            Authorization => 'Basic '
              . MIME::Base64::encode_base64( 'username:wrong', '' )
        }
    )->get( TestEnv->endpoint );

    ok !$response->{success};
    is $response->{status}, 401;
};

done_testing;
