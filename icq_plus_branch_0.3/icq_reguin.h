#ifndef __ICQ_REGUIN_H
#define __ICQ_REGUIN_H

int icq_regNewUin;
char icq_regpassword[10];
DWORD icq_regedUin;
HWND hwndRegImageDialog;

BOOL bImageRequested; //need to simply connect to ICQ if network error during registering

void InitReguin();
void icq_requestRegImage();
void icq_registerNewUin(char* password, char* image);

#endif /* __ICQ_REGUIN_H */
