#!/usr/bin/perl
# apt-get install libxml-simple-perl cppcheck rats
 
use strict;
use Data::Dumper;
use XML::Simple;

sub launchCPPCHECK();
sub launchRATS();

#

my %stats = ('info' => 0, 'warning' => 0, 'error' => 0);

launchCPPCHECK();
launchRATS();

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
	my %kstats = ('error' => 'error', 'style' => 'warning', 'information' => 'info', 'performance' => 'info');
	my %sevs = ('error' => 'bg-danger', 'style' => 'bg-warning', 'information' => 'bg-info', 'performance' => 'bg-info');

	print STDERR "# cppcheck " . $ref->{'cppcheck'}->{'version'} . "\n";
	if (defined($ref->{'errors'}->{'error'}))
	{
		my @errors = @{ $ref->{'errors'}->{'error'} };
		
		foreach my $error (@errors)
		{
			$stats{$kstats{$error->{'severity'}}}++;
			print STDERR "* Severity: " . $error->{'severity'} . "\n";
			print STDERR "  * Message: `" . $error->{'msg'} . "`\n";
			print STDERR "  * File: " . $error->{'location'}->{'file'} . ':' . $error->{'location'}->{'line'} . "\n" if (defined($error->{'location'}->{'file'}));
		}
	}
	print STDERR "\n";
	print " * xml version: " . $ref->{'version'} . "\n";	
	#print Dumper($ref);
}

sub launchRATS()
{
	print "Analyze 'rats'\n";
	my $res = `rats --xml --warning 1 . 2>&1`;
	my $ref = XMLin($res, ForceArray => ['vulnerability', 'file', 'line'], KeyAttr => 0);
	my %kstats = ('High' => 'error', 'Medium' => 'warning', 'Low' => 'info');
	my %sevs = ('High' => 'bg-danger', 'Medium' => 'bg-warning', 'Low' => 'bg-info');
	
	print STDERR "# rats\n";
	if (defined($ref->{'vulnerability'}))
	{
		my @vulnerabilities = @{ $ref->{'vulnerability'} };
		
		foreach my $vulnerability (@vulnerabilities)
		{
			my @files = @{ $vulnerability->{'file'} };
			
			print STDERR "* Severity: " . $vulnerability->{'severity'} . "\n";
			print STDERR "  * Type: " . $vulnerability->{'type'} . "\n";
			print STDERR "  * Message: `" . $vulnerability->{'message'} . "`\n";
			foreach my $file (@files)
			{
				my @lines = @{ $file->{'line'} };
				
				$stats{$kstats{$vulnerability->{'severity'}}}++;
				foreach my $line (@lines)
				{
					print STDERR "  * File:" . $file->{'name'} . ':' . $line . "\n";
				}
			}
		}
	}
	print STDERR "\n";
	print " * total_time: " . $ref->{'timing'}->{'total_time'} . "\n";
	print " * total_lines: " . $ref->{'timing'}->{'total_lines'} . "\n";
	print " * lines_per_second: " . $ref->{'timing'}->{'lines_per_second'} . "\n";
	#print Dumper($ref);
}
