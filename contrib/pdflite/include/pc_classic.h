/*---------------------------------------------------------------------------*
 |              PDFlib - A library for generating PDF on the fly             |
 +---------------------------------------------------------------------------+
 | Copyright (c) 1997-2006 Thomas Merz and PDFlib GmbH. All rights reserved. |
 +---------------------------------------------------------------------------+
 |                                                                           |
 |    This software is subject to the PDFlib license. It is NOT in the       |
 |    public domain. Extended versions and commercial licenses are           |
 |    available, please check http://www.pdflib.com.                         |
 |                                                                           |
 *---------------------------------------------------------------------------*/

/* $Id: pc_classic.h 9529 2012-07-08 07:09:28Z andijahja $
 *
 * Header for CodeWarrior to activate Classic builds (without CarbonLib).
 *
 */

/*
 * This must only be set for Classic builds. It is not used for the
 * standard build which is based on CarbonLib.
 */

#define PDF_TARGET_API_MAC_CLASSIC
