/*
 * $Id: hbppcalc.c 10241 2018-08-18 18:58:28Z lculik $
 */

/*
 * xHarbour Project source code:
 * PreProcessor support parser for #if and #elif constant expressions
 *
 * Copyright 2004 Ron Pinkas <ron @ xHarbour.com>
 * www - http://www.xharbour.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/).
 *
 */

/* Pacify MSVS2005 and above */
#if defined(_MSC_VER) && (_MSC_VER>=1400) &&(!defined(_CRT_SECURE_NO_WARNINGS))
   #define _CRT_SECURE_NO_WARNINGS
   #define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct tagBIOP
{
   char           Operator[3];
   double         Left;
   struct tagBIOP *Right;
} BIOP, *PBIOP;

PBIOP BiOp( void );
char * NextTokenInConstant( char **sExp );
static int Precedence( PBIOP Exp );
static double Reduce( PBIOP Exp );
double CalcConstant( char **pExp );
#define VALUE( sToken ) ( (sToken)[0] == '0' && (sToken)[1] == 'x' ? (double) strtol( sToken, NULL, 0 ) : atof( sToken ) )

#ifdef STAND_ALONE
  #define hb_compGenError( aArray, cType, sMsg, sInfo, sMore ) printf( "Parse Error: >%s<\n", sInfo )
  #include "hbexemem.h"

  int main( char argc, char *argv[] )
  {
     double dExp;
     char *sExp = argv[1];

     dExp = CalcConstant( (char **) &sExp );

     if( sExp[0] )
     {
        printf( "Parse Error: >%s<\n", sExp );
     }
     else
     {
        printf( "Result: %i\n", (int)dExp );
     }

     return 0;
  }
#else
   #include "hbppdef.h"
   #include "hbcomp.h"

   #define malloc( p ) hb_xgrab( p )
   #define free( p )   hb_xfree( p )
   #include "hbexemem.h"
#endif

double CalcConstant( char **pExp )
{
   char *sToken;
   PBIOP Exp = BiOp(), Root = Exp;
   double dExp;
   int bNot = 0;

   //printf( "Process: >%s<\n", *pExp );

  Top:

   sToken = NextTokenInConstant( pExp );

   if( sToken[0] == '(' )
   {
      if( bNot )
      {
         bNot = 0;
         Exp->Left = ! CalcConstant( pExp );
      }
      else
      {
         Exp->Left = CalcConstant( pExp );
      }

      sToken = NextTokenInConstant( pExp );

      if( sToken[0] != ')' )
      {
         *pExp -= strlen( sToken );
         hb_compGenError( hb_pp_szErrors, 'F', HB_PP_ERR_INVALID_CONSTANT_EXPRESSION, *pExp, NULL );
      }
   }
   else if( sToken[0] == '!' )
   {
      bNot = ! bNot;
      goto Top;
   }
   else
   {
      if( bNot )
      {
         bNot = 0;
         Exp->Left = ! VALUE( sToken );
      }
      else
      {
         Exp->Left = VALUE( sToken );
      }
   }

   sToken = NextTokenInConstant( pExp );

   if( sToken[0] && strstr( "+;-;*;/;&;|;&&;||;==;!=;<;>;<=;>=", sToken ) )
   {
      strcpy( Exp->Operator, sToken );

      Exp->Right = BiOp();

      Exp = Exp->Right;

      goto Top;
   }

   *pExp -= strlen( sToken );

   dExp = Reduce( Root );

   //printf( "Result: %i Next: >%s<\n", (int)dExp, *pExp );

   return dExp;
}

static double Reduce( PBIOP Exp )
{
   PBIOP Right;
   double dRet = 0;

   if( Exp->Operator[0] == 0 )
   {
      dRet = Exp->Left;

      free( (void *) Exp );

      return dRet;
   }

   if( Exp->Right->Operator[0] && Precedence( Exp ) >= Precedence( Exp->Right ) )
   {
      Right = Exp->Right;

      Exp->Right = BiOp();
      Exp->Right->Left = Right->Left;

      Right->Left = Reduce( Exp );

      return Reduce( Right );
   }

   //printf( "Operator: %s Left: %i Right: %i\n", Exp->Operator, (int) Exp->Left, (int) Reduce( Exp->Right ) );

   switch( Exp->Operator[0] )
   {
      case '+':
         dRet = Exp->Left + Reduce( Exp->Right );
         break;

      case '-':
         dRet = Exp->Left - Reduce( Exp->Right );
         break;

      case '*':
         dRet = Exp->Left * Reduce( Exp->Right );
         break;

      case '/':
         dRet = Exp->Left / Reduce( Exp->Right );
         break;

      case '&':
         if( Exp->Operator[1] )
         {
            if( Exp->Left )
            {
               dRet = Reduce( Exp->Right ) ? 1 : 0;
            }
            else
            {
               dRet = 0;
            }
         }
         else
         {
            dRet = (long) Exp->Left & (long) Reduce( Exp->Right );
         }
         break;

      case '|':
         if( Exp->Operator[1] )
         {
            if( Exp->Left )
            {
               dRet = 1;
            }
            else
            {
               dRet = Reduce( Exp->Right ) ? 1 : 0;
            }
         }
         else
         {
            dRet = (long) Exp->Left | (long) Reduce( Exp->Right );
         }
         break;

      case '=':
         dRet = (long) Exp->Left == (long) Reduce( Exp->Right );
         break;

      case '!':
         dRet = (long) Exp->Left != (long) Reduce( Exp->Right );
         break;

      case '<':
         if( Exp->Operator[1] )
         {
            dRet = (long) Exp->Left <= (long) Reduce( Exp->Right );
         }
         else
         {
            dRet = (long) Exp->Left < (long) Reduce( Exp->Right );
         }
         break;

      case '>':
         if( Exp->Operator[1] )
         {
            dRet = (long) Exp->Left >= (long) Reduce( Exp->Right );
         }
         else
         {
            dRet = (long) Exp->Left > (long) Reduce( Exp->Right );
         }
         break;
   }

   free( (void *) Exp );

   return dRet;
}

char * NextTokenInConstant( char **pExp )
{
   static char sToken[32];

   //printf( "Process: >%s<\n", *pExp );

   sToken[0] = '\0';

   if( *pExp[0] == '\0' )
   {
      return sToken;
   }

   while( (*pExp)[0] == ' ' )
   {
      (*pExp)++;
   }

   // Numbers
   if( isdigit( ( BYTE ) (*pExp)[0] ) || (*pExp)[0] == '-' || (*pExp)[0] == '+' )
   {
      int i = 0;

      if( (*pExp)[0] == '0' && (*pExp)[1] == 'x' )
      {
         sToken[0] = (*pExp)[0];
         sToken[1] = (*pExp)[1];
         (*pExp) += 2;

         i = 2;

         // Hex
         while( i < 31 && isxdigit( ( BYTE ) (*pExp)[0] ) )
         {
            sToken[i++] = (*pExp)[0];
            (*pExp)++;
         }

         sToken[i] = '\0';
         return sToken;
      }
      else
      {
         sToken[0] = '\0';

         while( (*pExp)[0] == '-' || (*pExp)[0] == '+' )
         {
            if( (*pExp)[0] == '-' )
            {
               if( sToken[0] == '-' )
               {
                  sToken[0] = '\0';
                  i = 0;
               }
               else
               {
                  sToken[0] = '-';
                  i = 1;
               }
            }
            else
            {
              // Postitive is already implied!
            }

            (*pExp)++;
         }
      }

      // Number
      do
      {
         sToken[i++] = (*pExp)[0];
         (*pExp)++;
      } while( i < 31 && isdigit( ( BYTE ) (*pExp)[0] ) );

      // Decimals
      if( i < 31 && (*pExp)[0] == '.' )
      {
         sToken[i++] = (*pExp)[0];
         (*pExp)++;

         while( i < 31 && isdigit( ( BYTE ) (*pExp)[0] ) )
         {
            sToken[i++] = (*pExp)[0];
            (*pExp)++;
         }
      }

      sToken[i] = '\0';
   }
   // Operators
   else if( (*pExp)[0] == '<' && (*pExp)[1] == '>' )
   {
      sToken[0] = '!';
      sToken[1] = '=';
      sToken[2] = '\0';

      (*pExp) += 2;
   }
   else if( strchr( "+-*/&|()!=<>", (*pExp)[0] ) )
   {
      sToken[0] = (*pExp)[0];
      sToken[1] = '\0';

      (*pExp)++;
   }
   else if( (*pExp)[0] == '#' )
   {
      sToken[0] = '!';
      sToken[1] = '=';
      sToken[2] = '\0';

      (*pExp)++;
   }
   else if( (*pExp)[0] == '.' && toupper( (*pExp)[1] ) == 'A' && toupper( (*pExp)[2] ) == 'N' && toupper( (*pExp)[3] ) == 'D' && toupper( (*pExp)[4] ) == '.' )
   {
      sToken[0] = '&';
      sToken[1] = '&';
      sToken[2] = '\0';

      (*pExp) += 5;
   }
   else if( (*pExp)[0] == '.' && toupper( (*pExp)[1] ) == 'N' && toupper( (*pExp)[2] ) == 'O' && toupper( (*pExp)[3] ) == 'T' && toupper( (*pExp)[4] ) == '.' )
   {
      sToken[0] = '!';
      sToken[1] = '\0';

      (*pExp) += 5;
   }
   else if( (*pExp)[0] == '.' && toupper( (*pExp)[1] ) == 'O' && toupper( (*pExp)[2] ) == 'R' && toupper( (*pExp)[3] ) == '.' )
   {
      sToken[0] = '|';
      sToken[1] = '|';
      sToken[2] = '\0';

      (*pExp) += 4;
   }
   else if( (*pExp)[0] == '.' && toupper( (*pExp)[1] ) == 'T' && toupper( (*pExp)[2] ) == '.' )
   {
      sToken[0] = '1';
      sToken[1] = '\0';

      (*pExp) += 3;
   }
   else if( (*pExp)[0] == '.' && toupper( (*pExp)[1] ) == 'F' && toupper( (*pExp)[2] ) == '.' )
   {
      sToken[0] = '0';
      sToken[1] = '\0';

      (*pExp) += 3;
   }
   else if( isalpha( ( BYTE ) (*pExp)[0] ) || (*pExp)[0] == '_' )
   {
      int i = 0;

      do
      {
         sToken[i++] = (*pExp)[0];
         (*pExp)++;
      } while( isalnum( ( BYTE ) (*pExp)[0] ) || (*pExp)[0] == '_' );
   }

   // Might be a first char of a double char operator!
   if( sToken[1] == '\0' &&
       ( ( sToken[0] == '&' && (*pExp)[0] == '&' ) ||
         ( sToken[0] == '|' && (*pExp)[0] == '|' ) ||
         ( sToken[0] == '=' && (*pExp)[0] == '=' ) ||
         ( sToken[0] == '!' && (*pExp)[0] == '=' ) ||
         ( sToken[0] == '<' && (*pExp)[0] == '=' ) ||
         ( sToken[0] == '>' && (*pExp)[0] == '=' ) )
     )
   {
      sToken[1] = (*pExp)[0];
      sToken[2] = '\0';

      (*pExp)++;
   }
   else if( sToken[0] == '\0' && (*pExp)[0] )
   {
      hb_compGenError( hb_pp_szErrors, 'F', HB_PP_ERR_INVALID_CONSTANT_EXPRESSION, *pExp, NULL );
   }
   else
   {
      //printf( "Token: >%s< Rest: >%s<\n", sToken, *pExp );
   }

   return sToken;
}

static int Precedence( PBIOP Exp )
{
   switch( Exp->Operator[0] )
   {
      case '|':
         if( Exp->Operator[1] )
         {
            return 1;
         }
         else
         {
           return 3;
         }

      case '&':
         if( Exp->Operator[1] )
         {
            return 2;
         }
         else
         {
            return 4;
         }

      case '=':
      case '!':
         return 5;

      case '<':
      case '>':
         return 6;

      case '+':
      case '-':
         return 7;

      case '*':
      case '/':
         return 8;
   }

   return 0;
}

PBIOP BiOp( void )
{
   PBIOP pBiOp = (PBIOP) malloc( sizeof( BIOP ) );

   memset( pBiOp, 0, sizeof( BIOP ) );

   return pBiOp;
}
