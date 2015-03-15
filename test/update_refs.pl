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
        my $test_ref_dir = "$test_dir" . "/reference/";
        my $tracefile = trim($splitted[1]);
        my $frames = defined($splitted[2]) ? trim($splitted[2]) : '';
        my $offset = defined($splitted[3]) ? trim($splitted[3]) : '';
	my $tolerance = defined($splitted[4]) ? trim($splitted[4]) : 0;
	my $full_test_path = "$test_path" . "$test_dir";
	my $result;

        if (! -d $full_test_path || ! -d "$full_test_path" . "/reference/")
	{
		printf("$test_dir test not found (or reference directory)\n");	
	}
	else
	{
	        my $intersection;
        	my $difference;

        	my @frames1 = glob("$test_ref_dir/frame????.ppm");
        	my @frames2 = glob("$test_dir/frame????.ppm");
 
        	my $passed = 1;
 
        	@{$intersection} = @{$difference} = ();
        	my %count = ();
        	my $element;
        	my $noise;
 
        	foreach $element (@frames1, @frames2) {
                	$count{basename($element)}++
        	}
        	foreach $element (keys %count) {
                	push @{ $count{$element} > 1 ? $intersection : $difference }, $element;
        	}
 
 
        	if(@{$difference} > 0) {
                	$passed = 0;
                	print("Error: missing frames:\n");
 
                	foreach $element (@{$difference}) {
                        	print("$element\n");
                	}
        	}
 
                chdir($full_test_path);
                printf("Entering at $test_dir ..\n");

        	foreach $element (@{$intersection})
        	{
                	`cp -i $element reference/$element`;
		}
                
		`cp -i stats.frames.csv.gz reference/stats.frames.csv.gz`;
                `cp -i output.txt reference/output.txt`;
	}
        chdir($test_path);
}

