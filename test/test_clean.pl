#!/usr/bin/perl
use strict;
use warnings;
use Cwd;
use File::Copy;
use File::Basename;
use File::Path;
use Cwd 'abs_path';


sub trim($)
{
        my $string = shift;
        $string =~ s/^\s+//;
        $string =~ s/\s+$//;
        return $string;
}

my $command;
my $binary_path;
my $test_path = abs_path(dirname($0)) . "/";
my $gpu3d_path = abs_path("$test_path/..");

my $regression_list_path = "$test_path/regression_list";
my $result_file_path = "$test_path/regression.out";

open(TESTS, $regression_list_path) or die 'ERROR: Cannot open $regression_list_path';
my @lines = <TESTS>;
close(TESTS);
my $line;

foreach $line (@lines) 
{
        my @splitted = split(/,/, $line);
        my $test_dir = trim($splitted[0]);
        my $tracefile = trim($splitted[1]);
        my $frames = defined($splitted[2]) ? trim($splitted[2]) : '';
        my $offset = defined($splitted[3]) ? trim($splitted[3]) : '';
	my $tolerance = defined($splitted[4]) ? trim($splitted[4]) : 0;
	my $full_test_path = "$test_path" . "$test_dir";
	my $result;

        if (! -d $full_test_path)
	{
		printf("$test_dir test not found\n");	
	}
	else
	{
        	chdir($full_test_path);
		`rm -f *.ppm`;
		`rm output.txt`;
		`rm stats*.*.*`;
	}
        chdir($test_path);
}

`rm -f $result_file_path`;

