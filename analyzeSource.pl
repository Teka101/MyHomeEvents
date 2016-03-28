#!/usr/bin/perl
# apt-get install libxml-simple-perl cppcheck rats
 
use strict;
use Data::Dumper;
use XML::Simple;

sub launchCPPCHECK();
sub launchRATS();

#

my %stats = ('info' => 0, 'warning' => 0, 'error' => 0);

print STDERR "<!DOCTYPE html>\n";
print STDERR "<head><title>Reporting</title>";
print STDERR '<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css" integrity="sha384-1q8mTJOASx8j1Au+a5WDVnPi2lkFfwwEAa8hDDdjZlpLegxhjVME1fgjWPGmkzs7" crossorigin="anonymous">';
print STDERR "</head>\n";
print STDERR "<body>\n";
launchCPPCHECK();
launchRATS();
print STDERR "</body>\n";

print "\n";
print "ERROR: " . $stats{'error'} . "\n";
print "WARNING: " . $stats{'warning'} . "\n";
print "INFO: " . $stats{'info'} . "\n";

exit(0);
 
###
 
sub launchCPPCHECK()
{
	print "Analyze 'cppcheck'\n";
	my $res = `cppcheck -q --enable=all --std=posix --xml --xml-version=2 . 2>&1`;
	my $ref = XMLin($res, ForceArray => ['error'], KeyAttr => 0);
	my %kstats = ('error' => 'error', 'style' => 'warning', 'information' => 'info');
	my %sevs = ('error' => 'bg-danger', 'style' => 'bg-warning', 'information' => 'bg-info');

	print STDERR "<h1>cppcheck " . $ref->{'cppcheck'}->{'version'} . "</h1>\n";
	if (defined($ref->{'errors'}->{'error'}))
	{
		my @errors = @{ $ref->{'errors'}->{'error'} };
		
		foreach my $error (@errors)
		{
			$stats{$kstats{$error->{'severity'}}}++;
			print STDERR "<div class=\"" . $sevs{$error->{'severity'}} . "\"><ul>";
			print STDERR "<li>Severity: " . $error->{'severity'} . "</li>"; #error / style / information
			print STDERR "<li>Message: <pre>" . $error->{'msg'} . "</pre></li>";
			print STDERR "<li>File: " . $error->{'location'}->{'file'} . ':' . $error->{'location'}->{'line'} . "</li>" if (defined($error->{'location'}->{'file'}));
			print STDERR "</ul></div>\n";
		}
	}
	print " * xml version: " . $ref->{'version'} . "\n";	
	#print Dumper($ref);
}

sub launchRATS()
{
	print "Analyze 'rats'\n";
	my $res = `rats --xml --warning 3 . 2>&1`;
	my $ref = XMLin($res, ForceArray => ['vulnerability', 'file', 'line'], KeyAttr => 0);
	my %kstats = ('High' => 'error', 'Medium' => 'warning', 'Low' => 'info');
	my %sevs = ('High' => 'bg-danger', 'Medium' => 'bg-warning', 'Low' => 'bg-info');
	
	print STDERR "<h1>rats</h1>\n";
	if (defined($ref->{'vulnerability'}))
	{
		my @vulnerabilities = @{ $ref->{'vulnerability'} };
		
		foreach my $vulnerability (@vulnerabilities)
		{
			my @files = @{ $vulnerability->{'file'} };
			
			$stats{$kstats{$vulnerability->{'severity'}}}++;
			print STDERR "<div class=\"" . $sevs{$vulnerability->{'severity'}} . "\"><ul>";
			print STDERR "<li>Severity: " . $vulnerability->{'severity'} . "</li>"; #High / Medium / Low
			print STDERR "<li>Type: " . $vulnerability->{'type'} . "</li>";
			print STDERR "<li>Message: <pre>" . $vulnerability->{'message'} . "</pre></li>";
			foreach my $file (@files)
			{
				my @lines = @{ $file->{'line'} };
				
				foreach my $line (@lines)
				{
					print STDERR "<li>File:" . $file->{'name'} . ':' . $line . "</li>";
				}
			}
			print STDERR "</ul></div>\n";
		}
	}
	print " * total_time: " . $ref->{'timing'}->{'total_time'} . "\n";
	print " * total_lines: " . $ref->{'timing'}->{'total_lines'} . "\n";
	print " * lines_per_second: " . $ref->{'timing'}->{'lines_per_second'} . "\n";
	#print Dumper($ref);
}