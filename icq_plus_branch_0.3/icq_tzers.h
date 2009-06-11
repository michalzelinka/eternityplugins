
CALLBACK tZersWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef enum
{
	TZER_NAME1 = 11,
	TZER_COUNT,
} TZerID;

typedef struct
{
	char *szTxt;
	char *szId;
	char *szUrl;
} TZerInfo;

TZerInfo tZers[TZER_COUNT];


