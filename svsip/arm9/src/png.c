/*
 * SvSIP, SIP/VoIP client for Nintendo DS
 * Copyright (C) 2007-2009  Samuel Vinson <samuelv0304@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <nds.h>
#include <png.h>

#define RGBA5551(r,g,b,a) ((r)|((g)<<5)|((b)<<10) | ((a)<<15))

ushort png2bmp(char *file_name, ushort **bmp, int* wp, int *hp) 
{
	png_uint_32 width, height;
	int bitDepth, colorType;
	int number_passes, pass, y,x;
	//png_byte **png_rows = 0, *pngOff;
	png_byte *png_row = NULL;
	ushort* bmpOff;
	int R, G, B,A;
	
	FILE *fp;

	if ((fp = fopen(file_name, "rb")) == NULL)
	{
      return 0;
	}
	
	//fileBytes = src;
	png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
	png_infop info_ptr = png_create_info_struct( png_ptr );
	
	if (setjmp(png_jmpbuf(png_ptr))) 
	{
		fclose(fp);
		return 0;
	} 
	
	//png_set_read_fn( png_ptr, 0, readPngCallback );	
	png_init_io(png_ptr, fp);
	
	png_read_info( png_ptr, info_ptr );
	
	png_get_IHDR( png_ptr, info_ptr, &width, 
		      &height, 
		      &bitDepth, 
		      &colorType, 0, 0, 0);
		      
	if (wp) *wp = width;
	if (hp) *hp = height;
	
	//printf("width %d height %d\n", width, height);
	
	//if (height != 96 || width != 96) return 0;
		
	if ( bitDepth == 16 )
		png_set_strip_16(png_ptr);
		
	if ( colorType == PNG_COLOR_TYPE_PALETTE )
		png_set_expand( png_ptr );
		
	if ( bitDepth < 8 )
		png_set_expand( png_ptr );
		
	if ( png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) )
		png_set_expand( png_ptr );
		
	if ( colorType == PNG_COLOR_TYPE_GRAY) {
		png_set_gray_to_rgb( png_ptr );
	}
	
//	if ( colorType & PNG_COLOR_MASK_ALPHA ) {
//		png_set_strip_alpha( png_ptr );
//	}
    
    png_color_8 sig_bit = {5 , 5, 5, 0, 1};
    png_set_shift(png_ptr, &sig_bit);
		
	number_passes = png_set_interlace_handling(png_ptr);
	
	png_read_update_info( png_ptr, info_ptr );
	
	bmpOff = *bmp = (ushort *)malloc(height * width * sizeof(ushort));

	png_row = png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));
	
    if ( colorType & PNG_COLOR_MASK_ALPHA )
    {
    	for (pass = 0; pass < number_passes; pass++)
        {
          for (y = 0; y < height; y++)
          {
    		 png_read_row(png_ptr, png_row, NULL);
    		 for(x = 0; x < width; ++x)
    		 {
                R = png_row[4*x];
                G = png_row[4*x+1];
                B = png_row[4*x+2];
                A = png_row[4*x+3];
                *bmpOff = RGBA5551(R,G,B,A);
    			bmpOff++;
    		 }
          }
    	}
    }
    else
    {
        for (pass = 0; pass < number_passes; pass++)
        {
          for (y = 0; y < height; y++)
          {
             png_read_row(png_ptr, png_row, NULL);
             for(x = 0; x < width; ++x)
             {
                R = png_row[3*x];
                G = png_row[3*x+1];
                B = png_row[3*x+2];
                *bmpOff = RGB5(R,G,B) | BIT(15);
                bmpOff++;
             }
          }
        }
    }

	png_free(png_ptr, png_row);
	
	png_destroy_read_struct( &png_ptr, &info_ptr, png_infopp_NULL );

	fclose(fp);
	
	return 1;
}
