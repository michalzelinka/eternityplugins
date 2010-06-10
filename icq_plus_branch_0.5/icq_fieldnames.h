// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright � 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright � 2001-2002 Jon Keating, Richard Hughes
// Copyright � 2002-2004 Martin �berg, Sam Kothari, Robert Rainwater
// Copyright � 2004-2009 Joe Kucera
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// -----------------------------------------------------------------------------
//
// File name      : $URL: http://sss.chaoslab.ru:81/svn/icqjplus/branches/0.5_branch/icq_fieldnames.h $
// Revision       : $Revision: 298 $
// Last change on : $Date: 2009-06-19 11:03:16 +0200 (Fri, 19 Jun 2009) $
// Last change by : $Author: persei $
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

struct FieldNamesItem 
{
  int code;
  char *text;
};

extern const FieldNamesItem countryField[];
extern const FieldNamesItem interestsField[];
extern const FieldNamesItem languageField[];
extern const FieldNamesItem pastField[];
extern const FieldNamesItem genderField[];
extern const FieldNamesItem agesField[];
extern const FieldNamesItem studyLevelField[];
extern const FieldNamesItem industryField[];
extern const FieldNamesItem occupationField[];
extern const FieldNamesItem affiliationField[];
extern const FieldNamesItem maritalField[];

char *LookupFieldName(const FieldNamesItem *table, int code);
char *LookupFieldNameUtf(const FieldNamesItem *table, int code, char *str, size_t strsize);