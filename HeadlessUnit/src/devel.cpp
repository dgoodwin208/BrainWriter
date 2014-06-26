/*
 * FIR filter class, by Mike Perkins
 * 
 * a simple C++ class for linear phase FIR filtering
 *
 * For background, see the post http://www.cardinalpeak.com/blog?p=1841
 *
 * Copyright (c) 2013, Cardinal Peak, LLC.  http://www.cardinalpeak.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1) Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2) Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 
 * 3) Neither the name of Cardinal Peak nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * CARDINAL PEAK, LLC BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "filt.h"

int main(int argc, char *argv[])
{
	FILE *fd_in, *fd_out;
	Filter *my_filter;
	short samp_dat;
	double out_val;
	int num_read;
	char outfile1[80] = "taps.txt";
	char outfile2[80] = "freqres.txt";

	my_filter = new Filter(LPF, 51, 44.1, 2.0);
	//my_filter = new Filter(HPF, 51, 44.1, 3.0);
	//my_filter = new Filter(BPF, 51, 44.1, 3.0, 6.0);

	fprintf(stderr, "error_flag = %d\n", my_filter->get_error_flag() );
	if( my_filter->get_error_flag() < 0 ) exit(1);
	my_filter->write_taps_to_file( outfile1 );
	my_filter->write_freqres_to_file( outfile2 );

	fd_in = fopen("cut.raw", "r");
	fd_out = fopen("filtered.raw", "w");

	while(1){
		num_read = fread(&samp_dat, sizeof(short), 1, fd_in);
		if(num_read != 1) break;
		out_val = my_filter->do_sample( (double) samp_dat );
		samp_dat = (short) out_val;
		fwrite(&samp_dat, sizeof(short), 1, fd_out);
	}

	fclose(fd_in);
	fclose(fd_out);
	delete my_filter;
}	
