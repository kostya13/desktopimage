void *OpenGraphic(char *name);
void DisplayGraphic(HWND hwnd,HDC pDC);
void CloseImage(void *Ipict);
void GetSize(int* x, int *y);

typedef struct tagImgInfo {
  IPicture *Ipic;
  SIZE sizeInHiMetric;
  SIZE sizeInPix;
  char *Path;
} IMG_INFO;

