// программы для открытия, отображени и закрытия изображения

// Author: Jacob Navia
//Here is a sample of what is possible to do with just plain C. This code will display in a window an image that can be in JPG, GIF, BMP, or Windows metafile format.

#include "ocidl.h"

#include "olectl.h"

#include "myimage.h"


static IMG_INFO ImageInfo;

void *OpenGraphic(char *name)
{
  IPicture *Ipic = NULL;
  SIZE sizeInHiMetric,sizeInPix;
  const int HIMETRIC_PER_INCH = 2540;
  HDC hDCScreen = GetDC(NULL);
  HRESULT hr;
  int nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
  int nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
  unsigned short OlePathName[512];

  ReleaseDC(NULL,hDCScreen);
  mbstowcs(OlePathName,name,strlen(name)+1);
  hr = OleLoadPicturePath(OlePathName,
        NULL,
        0,
        0,
        &IID_IPicture,
        (void *)(&Ipic));
  if (hr)
    return 0;
  if (Ipic) {
    // get width and height of picture

    hr = Ipic->lpVtbl->get_Width(Ipic,&sizeInHiMetric.cx);
    if (!SUCCEEDED(hr))
      goto err;
    Ipic->lpVtbl->get_Height(Ipic,&sizeInHiMetric.cy);
    if (!SUCCEEDED(hr))
      goto err;

    // convert himetric to pixels

    sizeInPix.cx = (nPixelsPerInchX * sizeInHiMetric.cx +
              HIMETRIC_PER_INCH / 2) / HIMETRIC_PER_INCH;
    sizeInPix.cy = (nPixelsPerInchY * sizeInHiMetric.cy +
              HIMETRIC_PER_INCH / 2) / HIMETRIC_PER_INCH;
    ImageInfo.sizeInPix = sizeInPix;
    ImageInfo.sizeInHiMetric = sizeInHiMetric;
    ImageInfo.Ipic = Ipic;
    ImageInfo.Path = name;

/*
	char buffer[65];
	itoa(ImageInfo.sizeInPix.cx,buffer,10);
	MessageBox(NULL,buffer, "caption", MB_OK);
*/
    return Ipic;

  }
err:
  return 0;
}

void DisplayGraphic(HWND hwnd,HDC pDC)
{
  IPicture *Ipic = ImageInfo.Ipic;
  DWORD dwAttr = 0;
  HBITMAP Bmp,BmpOld;
  RECT rc;
  HRESULT hr;
  HPALETTE pPalMemOld;

  if (Ipic != NULL)
  {
    // get palette

    OLE_HANDLE hPal = 0;
    HPALETTE hPalOld=NULL,hPalMemOld=NULL;
    hr = Ipic->lpVtbl->get_hPal(Ipic,&hPal);

    if (!SUCCEEDED(hr))
      return;
    if (hPal != 0)
    {
      hPalOld = SelectPalette(pDC,(HPALETTE)hPal,FALSE);
      RealizePalette(pDC);
    }


    GetClientRect(hwnd,&rc);

    // transparent?
    if (SUCCEEDED(Ipic->lpVtbl->get_Attributes(Ipic,&dwAttr)) ||
      (dwAttr &PICTURE_TRANSPARENT))

    {
      // use an off-screen DC to prevent flickering

      HDC MemDC = CreateCompatibleDC(pDC);
      Bmp =
       CreateCompatibleBitmap(pDC,ImageInfo.sizeInPix.cx,ImageInfo.sizeInPix.cy);

      BmpOld = SelectObject(MemDC,Bmp);
      pPalMemOld = NULL;
      if (hPal != 0)
      {
        hPalMemOld = SelectPalette(MemDC,(HPALETTE)hPal, FALSE);
        RealizePalette(MemDC);
      }
// Use this to show the left corner
      rc.left = rc.top = 0;
      rc.right = ImageInfo.sizeInPix.cx;
      rc.bottom = ImageInfo.sizeInPix.cy;

      // display picture using IPicture::Render

      hr = Ipic->lpVtbl->Render(Ipic,pDC,
      0,
      0,
      rc.right,
      rc.bottom,
      0,
      ImageInfo.sizeInHiMetric.cy,
      ImageInfo.sizeInHiMetric.cx,
      -ImageInfo.sizeInHiMetric.cy,
      &rc);

  /*    BitBlt(pDC,0, 0, ImageInfo.sizeInPix.cx,
                               ImageInfo.sizeInPix.cy,
        MemDC, 0, 0, SRCCOPY);
*/
      SelectObject(MemDC,BmpOld);

      if (pPalMemOld)  SelectPalette(MemDC,pPalMemOld, FALSE);
      DeleteObject(Bmp);
      DeleteDC(MemDC);

    }
/*    else
    {
      // display picture using IPicture::Render

      Ipic->lpVtbl->Render(Ipic,pDC,
       0,
       0,
       rc.right,
       rc.bottom,
       0,
       ImageInfo.sizeInHiMetric.cy,
       ImageInfo.sizeInHiMetric.cx,
       -ImageInfo.sizeInHiMetric.cy,
       &rc);
    }
*/
    if (hPalOld != NULL) SelectPalette(pDC,hPalOld, FALSE);
    if (hPal)  DeleteObject((HPALETTE)hPal);
  }
}

void CloseImage(void *Ipict)
{
  IPicture *ip = (IPicture *)Ipict;

  if (ip == NULL)
    ip = ImageInfo.Ipic;
  if (ip == NULL)
    return;
  ip->lpVtbl->Release(ip);
  memset(&ImageInfo,0,sizeof(ImageInfo));
}

void GetSize(int* x, int *y)
{
	*x=ImageInfo.sizeInPix.cx;
	*y=ImageInfo.sizeInPix.cy;
	return;
}
