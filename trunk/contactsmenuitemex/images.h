#ifndef __IMAGE_UTILS_H__
#define __IMAGE_UTILS_H__

//#define _WIN32_WINNT 0x0501
#include <windows.h>

void HalfBitmap32Alpha( HBITMAP );
void MakeBmpTransparent( HBITMAP );
void CorrectBitmap32Alpha( HBITMAP, BOOL );
HBITMAP CopyBitmapTo32( HBITMAP );
HBITMAP CreateBitmap32( int, int );
BOOL MakeBitmap32( HBITMAP * );
BOOL MakeGrayscale( HBITMAP * );
HICON MakeHalfAlphaIcon( HICON );
HICON MakeGrayscaleIcon( HICON );
HICON BindOverlayIcon( HICON, LPCSTR );

#endif // __IMAGE_UTILS_H__
