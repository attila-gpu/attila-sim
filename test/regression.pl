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
my $config_path;
my $test_path = abs_path(dirname($0)) . "/";
my $gpu3d_path = abs_path("$test_path/..");
my $comparison_tool = "icmp_diff";
my $script_path = abs_path("$test_path/script/");
my $exec_time_extract_tool = "$script_path/extract-exec-time.py";
my $exec_time_compare_tool = "$script_path/compare-exec-time.py";

my $regression_list_path = "$test_path/regression_list";
my $result_file_path = "$test_path/regression.out";

if(! -e "$gpu3d_path/bin/bGPU-Uni") {
	die "gpu3d binary not found in $gpu3d_path/bin/bGPU-Uni";
}
$binary_path = "$gpu3d_path/bin/bGPU-Uni";
if (! -e "$test_path/config/bGPU.ini"){
	die "gpu3d configuration file not found in $test_path/config/bGPU.ini";
}
$config_path = "$test_path/config";

open(TESTS, $regression_list_path) or die 'ERROR: Cannot open $regression_list_path';
my @lines = <TESTS>;
close(TESTS);
my $line;

open (RESULT, ">$result_file_path") or die 'ERROR: Cannot open $result_file_path';

sub compile
{
    my $source = shift;
    my $binary = shift;
    
    if (! -e $source){
       die "missing $comparison_tool source file";
    }
    print("Compiling comparison tool ($binary)...");
    `gcc -O $source -o $binary -lm`;
    if (! -e $binary)
    {
       die "Compilation failed";
    }
    else
    {
       print("Done\n");
    }
}

if (! -e "$test_path/$comparison_tool")
{
    # Compile comparison tool
    compile("$test_path/$comparison_tool.c","$test_path/$comparison_tool");
}
else
{
    # Check file age for necessary recompilation
    my $SourceFileAge = -M "$test_path/${comparison_tool}.c";
    my $BinaryFileAge = -M "$test_path/$comparison_tool";

    if ($SourceFileAge < $BinaryFileAge)
    {
       # Compile comparison tool
       compile("$test_path/$comparison_tool.c","$test_path/$comparison_tool");
    }
}

sub compare_execution_cycles {
        my $test_dir = shift;
        my $test_ref_dir = shift;
        my $start_frame = shift;
        my $extract_test_ref;
        my $extract_test;
        my $compare_result;

       # dump execution cycles columns into files
       $extract_test_ref = `$exec_time_extract_tool $test_ref_dir/stats.frames.csv.gz $script_path >$test_dir/exec_test_ref`;
       $extract_test = `$exec_time_extract_tool $test_dir/stats.frames.csv.gz $script_path >$test_dir/exec_test`;
       
       #compare execution cycles
       $compare_result = `$exec_time_compare_tool $test_dir/exec_test_ref $test_dir/exec_test $start_frame`;
       `rm $test_dir/exec_test_ref $test_dir/exec_test`;

       print("Execution cycles regression \"[Frame] SpeedUp\" : $compare_result\n");
       print RESULT ". [Frame] SpeedUp : $compare_result";
}

sub compare_results {
        my $test_dir = shift;
        my $test_ref_dir = shift;
        my $tolerance = shift;
        my $start_frame = shift;
	my $intersection;
        my $difference;
 
        my @frames1 = glob("$test_ref_dir/*.ppm");
        my @frames2 = glob("$test_dir/*.ppm");
 
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
                print("Result FAILED, missing frames:\n");
                print RESULT "FAILED, missing frames:\n"; # Print the results file

                foreach $element (@{$difference}) {
        		print("$element\n");
                        print RESULT "$element\n";
        	}
        }

        my @sorted_intersection = sort(@{$intersection});

        foreach $element (@sorted_intersection) 
        {
                my $cmd1 = "$test_path/$comparison_tool -i1 $test_ref_dir/$element -i2 $test_dir/$element -silent -od $test_dir/${element}_diff.ppm";
               
                my $cmd2 = "$test_path/$comparison_tool -i1 $test_ref_dir/$element -i2 $test_dir/$element -f \"%p\"";
 
                my $exit_value = system($cmd1);

                $noise = `$cmd2`;

                if ($exit_value > 0)
                {
                    $passed = 0;
                    print("Result FAILED, $comparison_tool exit != 0:\n");
                    print RESULT "FAILED, $comparison_tool exit != 0\n"; # Print the results file
                }
                else 
                {
                    if($noise < $tolerance && $noise != 0){
                        $passed = 0;
                        print("Result FAILED, $element psnr is $noise dB (below tolerated)\n");
                        print RESULT "FAILED, $element psnr is $noise dB (below tolerated)\n";
		    }
                    elsif ($noise != 0){
                        print("Result PASS: $element psnr is $noise dB (above tolerated)\n");
                        print RESULT "PASS: $element psnr is $noise dB (above tolerated) ";
                    }
                    else{ # noise = 0 
                        print("Result PASS: Output images $element are identical \n");
                        print RESULT "PASS: Output images $element are identical ";
                        `rm -f $test_dir/"$element"_diff.ppm`;
                    }
                }
        }

        if($passed) 
        {
                compare_execution_cycles($test_dir, $test_ref_dir, $start_frame);
        }
	
	print("\n");

	return $passed;
}

foreach $line (@lines) 
{
        my @splitted = split(/,/, $line);
        my $test_dir = trim($splitted[0]);
        my $configfile = trim($splitted[1]);
        my $tracefile = trim($splitted[2]);
        my $frames = defined($splitted[3]) ? trim($splitted[3]) : '';
        my $start_frame = defined($splitted[4]) ? trim($splitted[4]) : '';
	my $tolerance = defined($splitted[5]) ? trim($splitted[5]) : 0;
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
		print("Executing $test_dir...\n");
        	copy("$config_path/$configfile","./bGPU.ini") or die "ERROR: $configfile copy failed";
        	my $code;
        	$code = system("$binary_path $tracefile $frames $start_frame | tee output.txt");
        	if($code != 0) {
                	print("INTERRUPTED (returned code $code)\n");
                        print RESULT "INTERRUPTED (returned code $code)\n";
        	}
		system('rm bGPU.ini');
		
		if (! -d "$full_test_path/reference/")
		{
			die "ERROR: reference dir not found in $test_dir";
		}

 	        print("Testing Regression for $test_dir (reference tolerance is $tolerance dB)...\n");
                print RESULT "$test_dir (ref tolerance is $tolerance dB): ";

	
		$result = compare_results("$full_test_path", "$full_test_path/reference", $tolerance, $start_frame); 
		
		if ($result == 0) {
			die "REGRESSION TEST STOPPED\n"
	        }	
	}
        chdir($test_path);
}

close (RESULT);
