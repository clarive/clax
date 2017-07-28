use strict;
use warnings;
use lib 't/lib';

use Test::More;
use TestEnv;

subtest 'bad request on missing arg' => sub {
    my $ua = TestEnv->build_ua;
    my $response = $ua->post_form( TestEnv->endpoint . '/command', {} );

    ok !$response->{success};
    is $response->{status}, 400;

    like $response->{content}, qr/missing required field/i;
};

subtest 'runs command' => sub {
    my $ua = TestEnv->build_ua;
    my $response =
      $ua->post_form( TestEnv->endpoint . '/command', { command => 'echo' } );

    ok $response->{success};
    is $response->{status}, 200;

    is $response->{headers}->{'x-clax-status'}, 'success';
    like $response->{headers}->{'x-clax-pid'},  qr/\d+/;
    is $response->{headers}->{'x-clax-exit'},   '0';
};

subtest 'runs command with args' => sub {
    my $ua = TestEnv->build_ua;
    my $response =
      $ua->post_form( TestEnv->endpoint . '/command', { command => 'echo hi' } );

    ok $response->{success};
    is $response->{status}, 200;

    like $response->{content}, qr/hi/;

    is $response->{headers}->{'x-clax-status'}, 'success';
    like $response->{headers}->{'x-clax-pid'},  qr/\d+/;
    is $response->{headers}->{'x-clax-exit'},   '0';
};

subtest 'runs failed command' => sub {
    my $ua = TestEnv->build_ua;
    my $response =
      $ua->post_form( TestEnv->endpoint . '/command', { command => 'false' } );

    ok $response->{success};
    is $response->{status}, 200;

    is $response->{headers}->{'x-clax-status'}, 'error';
    like $response->{headers}->{'x-clax-pid'},  qr/\d+/;
    is $response->{headers}->{'x-clax-exit'},   '1';
};

done_testing;
