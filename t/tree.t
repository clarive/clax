use strict;
use warnings;
use lib 't/lib';

use Test::More;
use TestEnv;

subtest 'does not exist' => sub {
    my $ua       = TestEnv->build_ua;
    my $response = $ua->get( TestEnv->endpoint . '/tree/unknown' );

    ok !$response->{success};
    is $response->{status}, 404;
};

subtest 'uploads file' => sub {
    my $ua       = TestEnv->build_ua;
    my $response = $ua->post( TestEnv->endpoint . '/tree/test.file',
        { content => 'hello there' } );

    ok $response->{success};
    is $response->{status},  204;
    is $response->{content}, undef;

    $response = $ua->head( TestEnv->endpoint . '/tree/test.file' );

    ok $response->{success};
    is $response->{headers}->{'x-clax-crc32'},   '6d12e950';
    is $response->{headers}->{'content-length'}, '11';
    is $response->{content}, undef;
};

subtest 'uploads empty file' => sub {
    my $ua       = TestEnv->build_ua;
    my $response = $ua->post( TestEnv->endpoint . '/tree/empty.file',
        { content => '' } );

    ok $response->{success};
    is $response->{status},  204;
    is $response->{content}, undef;

    $response = $ua->head( TestEnv->endpoint . '/tree/empty.file' );

    ok $response->{success};
    is $response->{headers}->{'x-clax-crc32'},   '00000000';
    is $response->{headers}->{'content-length'}, '0';
    is $response->{content}, undef;
};

subtest 'creates directory deeply nested' => sub {
    my $ua       = TestEnv->build_ua;
    my $response = $ua->post(
        TestEnv->endpoint . '/tree/deeply/nested',
        {
            headers => { 'Content-Type' => 'application/vnd.clax.folder' }
        }
    );

    ok $response->{success};
    is $response->{status},  204;
    is $response->{content}, undef;

    $response = $ua->head( TestEnv->endpoint . '/tree/deeply/nested/' );

    ok $response->{success};
    is $response->{headers}->{'content-length'}, '0';
    is $response->{headers}->{'content-type'}, 'application/vnd.clax.folder';
    is $response->{content}, undef;
};

subtest 'uploads file deeply nested' => sub {
    my $ua       = TestEnv->build_ua;
    my $response = $ua->post( TestEnv->endpoint . '/tree/deeply/nested/test.file',
        { content => 'hello there' } );

    ok $response->{success};
    is $response->{status},  204;
    is $response->{content}, undef;

    $response = $ua->head( TestEnv->endpoint . '/tree/deeply/nested/test.file' );

    ok $response->{success};
    is $response->{headers}->{'content-length'}, '11';
    is $response->{content}, undef;
};

subtest 'uploads file setting time' => sub {
    my $ua = TestEnv->build_ua;
    my $response =
      $ua->post( TestEnv->endpoint . '/tree/test.file?time=1234567890',
        { content => 'hello there' } );

    ok $response->{success};
    is $response->{status},  204;
    is $response->{content}, undef;

    $response = $ua->head( TestEnv->endpoint . '/tree/test.file' );

    ok $response->{success};
    is $response->{headers}->{'last-modified'}, 'Fri, 13 Feb 2009 23:31:30 GMT';

    $response = $ua->delete( TestEnv->endpoint . '/tree/test.file' );

    ok $response->{success};
    is $response->{status},  204;
    is $response->{content}, undef;
};

subtest 'uploads file with invalid crc32' => sub {
    my $ua       = TestEnv->build_ua;
    my $response = $ua->post( TestEnv->endpoint . '/tree/test.file?crc=123',
        { content => 'hello there' } );

    ok !$response->{success};
    is $response->{status}, 400;

    $ua       = TestEnv->build_ua;
    $response = $ua->get( TestEnv->endpoint . '/tree/test.file' );

    ok !$response->{success};
    is $response->{status}, 404;
};

subtest 'uploads file with valid crc32' => sub {
    my $ua = TestEnv->build_ua;
    my $response =
      $ua->post( TestEnv->endpoint . '/tree/test.file?crc=6d12e950',
        { content => 'hello there' } );

    ok $response->{success};
    is $response->{status}, 204;

    $ua->delete( TestEnv->endpoint . '/tree/test.file' );
};

subtest 'not found when deleting unknown file' => sub {
    my $ua       = TestEnv->build_ua;
    my $response = $ua->delete( TestEnv->endpoint . '/tree/unlikely-to-exist' );

    ok !$response->{success};

    like $response->{content}, qr/not found/i;
};

done_testing;
