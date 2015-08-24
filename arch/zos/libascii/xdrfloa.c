#include <sys/types.h>
#include <stdio.h>
#include <float.h>

#ifdef GEN_PRAGMA_EXPORT
#pragma export(ConvertFloatToIEEE)
#pragma export(ConvertDoubleToIEEE)
#pragma export(ConvertIEEEToFloat)
#pragma export(ConvertIEEEToDouble)
#endif

static char ibmcopyr[] =
   "XDR_FLOA - Licensed Materials - Property of IBM. "
   "This module is \"Restricted Materials of IBM\" "
   "5735-FAL (C) Copyright IBM Corp. 1991. "
   "5735-HAL (C) Copyright IBM Corp. 1991. "
   "See IBM Copyright Instructions.";
                                                               
/*** IBMCOPYR ********************************************************/
/*                                                                   */
/* Copyright:                                                        */
/*   Licensed Materials - Property of IBM                            */
/*   This product contains "Restricted Materials of IBM"             */
/*   5735-FAL (C) Copyright IBM Corp. 1991.                          */
/*   5735-HAL (C) Copyright IBM Corp. 1991.                          */
/*   All rights reserved.                                            */
/*   US Government Users Restricted Rights -                         */
/*   Use, duplication or disclosure restricted by GSA ADP Schedule   */
/*   Contract with IBM Corp.                                         */
/*   See IBM Copyright Instructions.                                 */
/*                                                                   */
/* Status:                                                           */
/*   Version 2, Release 1, Level 0                                   */
/*                                                                   */
/*** IBMCOPYR ********************************************************/

#define MAXENVVALUE 256
/* Convert floating point numbers between IBM/370 and IEEE\                     
 representations                                                                
 *                                                                       
 * Synopsis:                                                                    
 *                                                                              
 *  void ConvertFloatToIEEE(src, dst)   -- convert Native float to IEEE float
 *  float *src, *dst;                                                         
 *                                                    
 *   void ConvertDoubleToIEEE(src, dst) -- convert native double to IEEE double 
 *    double *src, *dst;                                                        
 *                                                                              
 *    void ConvertIEEEToFloat(src, dst) -- convert IEEE float to Native float
 *    float *src, *dst;                                                         
 *                                                                              
 *    void ConvertIEEEToDouble(src, dst) -- convert IEEE double to Native double
 *    double *src, *dst;                                                        
 *                                                                              
 * Notes:                                                                       
 *                                                                              
 * Magnitudes which underflow during conversion are converted to 0.\            
 Magnitudes                                                                     
 * which overflow during conversion are converted to the largest\               
 representable                                                                  
 * number.                                                                      
 *                                                                              
 */                                                                             
#define CHECKOUT  0  /* compile as standalone program for debugging? */         
                                                                                
#define HI 0 /* index of hi order byte/word/long */                             
#define LO 1 /* index of lo order byte/word/long */                             
                                                                                
/* largest representable numbers, IEEE format */                                
#define fMAXIEEE   0x7F7FFFFF                                                   
#define dMAXIEEEhi 0x7FEFFFFF                                                   
#define dMAXIEEElo 0xFFFFFFFF                                                   
                                                                                
/* largest representable numbers, 370 format */                                 
#define fMAX370    0x7FFFFFFF                                                   
#define dMAX370hi  0x7FFFFFFF                                                   
#define dMAX370lo  0xFFFFFFFF                                                   
                                                                                
/* Summary of floating point representations                                    
 *                                                                              
 * ************************                                                     
 * IBM/370 single precision                                                     
 * ************************                                                     
 *                                                                              
 * xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx                                      
 *  |-exp--| |--------fraction-----------|                                      
 *    (7)               (24)                                                    
 *                                                                              
 *   value = (-1)**s * 16**(e - 64) * .f     range = 5E-79 ... 7E+75            
 *                                                                              
 * *********************                                                        
 * IEEE single precision                                                        
 * *********************                                                        
 *                                                                              
 * xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx                                      
 *  |--exp---||-------fraction-----------|                                      
 *     (8)              (23)                                                    
 *                                                                              
 *   value = (-1)**s * 2**(e - 127) * 1.f    range = 1E-38 ... 3E+38            
 *                                                                              
 * ************************                                                     
 * IBM/370 double precision                                                     
 * ************************                                                     
 *                                                                              
 * xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx yyyy.yyyy yyyy.yyyy\                 
 yyyy.yyyy yyyy.yyyy                                                            
 *  |-exp--| |-------------------------------fraction------------------\        
----------|                                                                     
 *    (7)                                      (56)                             
 *                                                                              
 *   value = (-1)**s * 16**(e - 64) * .f     range = 5E-79 ... 7E+75            
 *                                                                              
 * *********************                                                        
 * IEEE double precision                                                        
 * *********************                                                        
 *                                                                              
 * xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx yyyy.yyyy yyyy.yyyy\                 
 yyyy.yyyy yyyy.yyyy                                                            
 *  |--exponent-| |-------------------------fraction-------------------\        
----------|                                                                     
 *       (11)                                 (52)                              
 *                                                                              
 *   value = (-1)**s * 2**(e - 1023) * 1.f   range = 2E-308 ... 3E+308          
 */                                                                             
                                                                                
/* 64 bit shift left                                                            
 */                                                                             
static lshift(p, n)                                                             
unsigned long p[]; int n;                                                       
   {                                                                            
   register unsigned long hi, lo;                                               
                                                                                
   hi = p[0];                                                                   
   lo = p[1];                                                                   
                                                                                
   while (n-- > 0)                                                              
      {                                                                         
      hi = (hi << 1) | ((lo >> 31) & 0x00000001);                               
      lo <<= 1;                                                                 
      }                                                                         
                                                                                
   p[0] = hi;                                                                   
   p[1] = lo;                                                                   
   }                                                                            
                                                                                
/* 64 bit shift right                                                           
 */                                                                             
static rshift(p, n)                                                             
unsigned long p[]; int n;                                                       
   {                                                                            
   register unsigned long hi, lo;                                               
                                                                                
   hi = p[0];                                                                   
   lo = p[1];                                                                   
                                                                                
   while (n-- > 0)                                                              
      {                                                                         
      lo = (hi << 31) | ((lo >> 1) & 0x7fffffff);                               
      hi >>= 1;                                                                 
      }                                                                         
                                                                                
   p[0] = hi;                                                                   
   p[1] = lo;                                                                   
   }                                                                            
                                                                                
/* Convert IBM/370 "float" to IEEE "float"                                      
 */                                                                             
ConvertFloatToIEEE(src, dst)                                                     
unsigned long src[1], dst[1];                                                   
   {                                                                            
   unsigned long x, s, f;                                                       
   long e;                                                                      
                                                                                
   x = *src;                                                                    
                                                                                
   /* check for special case */                                                 
   if ((x & 0x7fffffff) == 0)                                                   
      {                                                                         
      *dst = x;                                                                 
      return; /* zero */                                                        
      }                                                                         
                                                                                
   /* fetch the sign, exponent (removing excess 64), and fraction */            
   s =   x & 0x80000000;                                                        
   e = ((x & 0x7f000000) >> 24) - 64;                                           
   f =   x & 0x00ffffff;                                                        
                                                                                
   /* convert scale factor from "16**exponent" to "2**exponent" format          
    * -- we simply multiply the exponent by 4                                   
    */                                                                          
   if (e >= 0)                                                                  
      e <<= 2;                                                                  
   else                                                                         
      e = -((-e) << 2);                                                         
                                                                                
   /* convert exponent for 24 bit fraction to 23 bit fraction */                
   e -= 1;                                                                      
                                                                                
   /* normalize the fraction */                                                 
   if (f)                                                                       
      while ((f & 0x00800000) == 0)                                             
         {                                                                      
         f <<= 1;                                                               
         e -= 1;                                                                
         }                                                                      
                                                                                
   /* drop the implied '1' preceeding the binary point */                       
   f &= 0x007fffff;                                                             
                                                                                
   /* convert exponent to excess 127 and store the number */                    
   if ((e += 127) >= 255)                                                       
      *dst = s | fMAXIEEE;                                                      
   else if (e <= 0)                                                             
     *dst = s;                                                                 
   else                                                                         
      *dst = s | (e << 23) | f;                                                 
   }                                                                            
                                                                                
/* Convert IBM/370 "double" to IEEE "double"                                    
 */                                                                             
ConvertDoubleToIEEE(src, dst)                                                     
unsigned long src[2], dst[2];                                                   
   {                                                                            
   unsigned long x, s, f[2];                                                    
   long e;                                                                      
                                                                                
   x = src[HI];                                                                 
                                                                                
   /* check for special case */                                                 
   if ((x & 0x7fffffff) == 0)                                                   
      {                                                                         
      dst[HI] = src[HI];                                                        
      dst[LO] = src[LO];                                                        
      return; /* zero */                                                        
      }                                                                         
                                                                                
   /* fetch the sign, exponent (removing excess 64), and fraction */            
   s =    x & 0x80000000;                                                       
   e =  ((x & 0x7f000000) >> 24) - 64;                                          
   f[0] = x & 0x00ffffff;                                                       
   f[1] = src[LO];                                                              
                                                                                
   /* convert scale factor from "16**exponent" to "2**exponent" format          
    * -- we simply multiply the exponent by 4                                   
    */                                                                          
   if (e >= 0)                                                                  
      e <<= 2;                                                                  
   else                                                                         
      e = -((-e) << 2);                                                         
                                                                                
   /* normalize the fraction (to 57 bits) */                                    
   if (f[0])                                                                    
      while ((f[0] & 0x01000000) == 0)                                          
         {                                                                      
         lshift(f, 1);                                                          
         e -= 1;                                                                
         }                                                                      
                                                                                
   /* convert 57 bit fraction to 53 bit fraction */                             
   rshift(f, 4);                                                                
                                                                                
   /* drop the implied '1' preceeding the binary point */                       
   f[0] &= 0x000fffff;                                                          
                                                                                
   /* convert exponent to excess 1023 and store the number */                   
   if ((e += 1023) >= 2047)                                                     
      {                                                                         
      dst[HI] = s | dMAXIEEEhi;                                                 
      dst[LO] =     dMAXIEEElo;                                                 
      }                                                                         
   else if (e <= 0)                                                             
      {                                                                         
      dst[HI] = s;                                                              
      dst[LO] = 0;                                                              
      }                                                                         
   else                                                                         
      {                                                                         
      dst[HI] = s | (e << 20) | f[0];                                           
      dst[LO] = f[1];                                                           
      }                                                                         
   }                                                                            
                                                                                
/* Convert IEEE "float" to IBM/370 "float"                                      
 */                                                                             
ConvertIEEEToFloat(src, dst)                                                     
unsigned long src[1], dst[1];                                                   
   {                                                                            
   unsigned long x, s, f;                                                       
   long e;                                                                      
   double oflow;
   char tmpenv[MAXENVVALUE];

   x = *src;                                                                    

   /* check for special case */                                                 
   if ((x & 0x7fffffff) == 0)                                                   
      {                                                                         
      *dst = x; 
      return; /* zero */                                                        
      }       

   /* Check for overflow condition.                              */
   /*                                                            */
   /* 0x4FAFFFFF FFFFFFFF is DBL_MAX for S370 Converted to IEEE. */
   /*                         OR                                 */
   /* ConvertDoubleToIEEE(DBL_MAX, maxIEEEdouble);               */
   /* Where max IEEEdouble would be set to 0x4FaFFFFF FFFFFFFF.  */
   /*                                                            */
   /* The following turns off the sign bit and tests for greater */
   /* than 0x4FAFFFFF.                                           */
   if ((unsigned long) (x & 0x7FFFFFFF) > (unsigned long) 0x4FAFFFFF)
      {                                                                         
	    oflow = DBL_MAX;
	    /* If the value is to big then for now cause an overflow so
	       IBM and customers will know that full IEEE floating point
	       operations are required for this application. */
	    oflow = oflow / 0.00001;
        return;
	  }
   /* fetch the sign, exponent (removing excess 127), and fraction */           
   s =   x & 0x80000000;                                                        
   e = ((x & 0x7f800000) >> 23) - 127;                                          
   f =   x & 0x007fffff;                                                        
                                                                                
   /* convert 23 bit fraction to 24 bit fraction */                             
   f <<= 1; /* >>fraction is now 24 bits<< */                                   
                                                                                
   /* restore the implied '1' which preceeded the IEEE binary point */          
   f |= 0x01000000; /* >>fraction is now 25 bits<< */                           
                                                                                
   /* convert scale factor from "2**exponent" to "16**exponent" format          
    * -- we adjust fraction and exponent in opposite directions until           
    * exponent is a multiple of 4, then divide the exponent by 4                
    */                                                                          
  if (e >= 0)                                                                   
      {                                                                         
      f <<= (e & 3);    /* >>fraction is now 28 bits, max<< */                  
      e >>= 2;                                                                  
      }                                                                         
   else                                                                         
      {                                                                         
      f >>= ((-e) & 3); /* >>fraction is now 25 bits, max<< */                  
      e = -((-e) >> 2);                                                         
      }                                                                         
                                                                                
   /* reduce fraction to 24 bits */                                             
   if (f & 0x0f000000)                                                          
      {                                                                         
      f >>= 4;                                                                  
      e += 1;                                                                   
      }                                                                         
                                                                                
   /* convert exponent to excess 64 and store the number */                     
   if ((e += 64) > 127)                                                         
      *dst = s | fMAX370;                                                       
   else if (e < 0)                                                              
      *dst = s;                                                                 
   else                                                                         
      *dst = s | (e << 24) | f;                                                 
   }                                                                            
                                                                                
/* Convert IEEE "double" to IBM/370 "double"                                    
 */                                                                             
ConvertIEEEToDouble(src, dst)                                                     
unsigned long src[2], dst[2];                                                   
   {                                                                            
   unsigned long x, s, f[2];                                                    
   long e;                                                                      
   double oflow;
   char tmpenv[MAXENVVALUE];

   x = src[HI];                                                                 

   /* check for special case */                                                 
   if ((x & 0x7fffffff) == 0)                                                   
      {                                                                         
      dst[HI] = src[HI];                                                        
      dst[LO] = src[LO];                                                        
      return; /* zero */                                                        
      }                                                                         

   /* Check for overflow condition.                              */
   /*                                                            */
   /* 0x4FAFFFFF FFFFFFFF is DBL_MAX for S370 Converted to IEEE. */
   /*                         OR                                 */
   /* ConvertDoubleToIEEE(DBL_MAX, maxIEEEdouble);               */
   /* Where max IEEEdouble would be set to 0x4FaFFFFF FFFFFFFF.  */
   /*                                                            */
   /* The following turns off the sign bit and tests for greater */
   /* than 0x4FAFFFFF.                                           */
   if ((unsigned long) (x & 0x7FFFFFFF) > (unsigned long) 0x4FAFFFFF)
      {                                                                         
	    oflow = DBL_MAX;
	    /* If the value is to big then for now cause an overflow so
	       IBM and customers will know that full IEEE floating point
	       operations are required for this application. */
	    oflow = oflow / 0.00001;
        return;
	  }

   /* fetch the sign, exponent (removing excess 1023), and fraction */          
   s =    x & 0x80000000;                                                       
   e =  ((x & 0x7ff00000) >> 20) - 1023;                                        
   f[0] = x & 0x000fffff;                                                       
   f[1] = src[LO];                                                              
                                                                                
   /* convert 52 bit fraction to 56 bit fraction */                             
   lshift(f, 4); /* >>fraction is now 56 bits<< */                              
                                                                                
   /* restore the implied '1' which preceeded the IEEE binary point */          
   f[0] |= 0x01000000; /* >>fraction is now 57 bits<< */                        
                                                                                
   /* convert scale factor from "2**exponent" to "16**exponent" format          
    * -- we adjust fraction and exponent in opposite directions until           
    * exponent is a multiple of 4, then divide the exponent by 4                
    */                                                                          
  if (e >= 0)                                                                   
      {                                                                         
      lshift(f, e & 3);    /* >>fraction is now 60 bits, max<< */               
      e >>= 2;                                                                  
      }                                                                         
   else                                                                         
      {                                                                         
      rshift(f, (-e) & 3); /* >>fraction is now 57 bits, max<< */               
      e = -((-e) >> 2);                                                         
      }                                                                         
                                                                                
   /* reduce fraction to 56 bits */                                             
   if (f[0] & 0x0f000000)                                                       
      {                                                                         
      rshift(f, 4);                                                             
      e += 1;                                                                   
      }                                                                         
                                                                                
   /* convert exponent to excess 64 and store the number */                     
   if ((e += 64) > 127)                                                         
      {                                                                         
      dst[HI] = s | dMAX370hi;                                                  
      dst[LO] =     dMAX370lo;                                                  
      }                                                                         
   else if (e < 0)                                                              
      {                                                                         
      dst[HI] = s;                                                              
      dst[LO] = 0;                                                              
      }                                                                         
   else                                                                         
      {                                                                         
      dst[HI] = s | (e << 24) | f[0];                                           
      dst[LO] = f[1];                                                           
      }                                                                         
   }                                                                            
                                                                                
