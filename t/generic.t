use strict;
use warnings;
use lib 't/lib';

use Test::More;
use TestEnv;

subtest 'ping request' => sub {
    my $ua       = TestEnv->build_ua;
    my $response = $ua->get( TestEnv->endpoint );

    ok $response->{success};

    is $response->{headers}->{'transfer-encoding'}, 'chunked';
    is $response->{headers}->{'content-type'},      'application/json';

    like $response->{content}, qr/version"\s*:\s*"\d+-[a-f0-9]{7}/;
    like $response->{content}, qr/os"\s*:\s*"/;
    like $response->{content}, qr/arch"\s*:\s*"/;
};

subtest 'not found' => sub {
    my $ua       = TestEnv->build_ua;
    my $response = $ua->get( TestEnv->endpoint . '/unknown' );

    ok !$response->{success};
    is $response->{status}, 404;
};

subtest 'method not allowed' => sub {
    my $ua       = TestEnv->build_ua;
    my $response = $ua->post( TestEnv->endpoint );

    ok !$response->{success};
    is $response->{status}, 405;
};

done_testing;
