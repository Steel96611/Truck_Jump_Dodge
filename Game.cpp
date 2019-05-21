#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9core.h>
#include <tchar.h>
#include <locale.h>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

// 以下はプロジェクトへの設定無しでライブラリをリンクするための記述
#pragma comment(lib, "d3d9.lib")	// Direct3D9 ライブラリをリンクする
#pragma comment(lib, "d3dx9.lib")	// Direct3DX9 ライブラリをリンクする

struct DataSet;
class MyBitmap;

// 関数宣言
HRESULT initDirect3D(DataSet *data);
const LPCTSTR d3dErrStr(HRESULT res);
HRESULT loadTexture(IDirect3DDevice9 *dev, LPCTSTR fname, IDirect3DTexture9 **tex);

// ウィンドウの幅と高さを定数で設定する
#define WIDTH 1520		//ウィンドウの幅
#define HEIGHT 800		//ウィンドウの高さ

// 配列に順番にそれぞれのオブジェクトに関する値を代入する
#define Player 0		//プレイヤー
#define Hoeel  1		//ホイール
#define Life   2		//ライフ
#define Block  3		//ブロック
#define Enemy  4		//エネミー

#define TITLE  0		//タイトルシーン
#define GAME   1		//ゲームシーン
#define RESULT 2		//リザルトシーン

/// プログラムに必要な変数を構造体として定義
struct DataSet {
	HINSTANCE hInstance;			//インスタンスハンドル
	HWND hWnd;						//表示ウィンドウ

	IDirect3D9 *pD3D;				//Direct3Dインスタンスオブジェクト
	D3DPRESENT_PARAMETERS d3dpp;	//デバイス作成時のパラメータ

	IDirect3DTexture9 *Back;		//スプライトに貼り付けるテクスチャ

	IDirect3DDevice9 *dev;			//Direct3Dデバイスオブジェクト
	ID3DXSprite *spr;				//スプライト

	MyBitmap *font;			//ビットマップ

	int spr_x, spr_y;		//スプライト表示位置

	int drow_x[10][10];		//書き出し始める座標のx軸
	int drow_y[10][10];		//書き出し始める座標のy軸
	int bit_x[10][10];		//描画する位置のx軸
	int bit_y[10][10];		//描画する位置のy軸
	int bit_width[10][10];	//表示する横幅
	int bit_height[10][10];	//表示する縦幅

	int Sclrol;				//背景のスクロールの値

	int Scroll_Reset;		//スクロールのリセット

	int Scene;				//ゲームのシーン

	int Spin;				//Playerの車輪の回転

	int Range;				//Enemyの座標計算に必要な値

	int life;				//残機

	int Speed;				//スクロールの速度

	bool jump;				//Playerの状態(飛んでいるかどうか)

} mydata;

//ビットマップで表示するためのクラス
class MyBitmap {
private:
	IDirect3DTexture9 *tex;
	static ID3DXSprite *spr;
	MyBitmap() :tex(0) {}		// コンストラクタの呼び出しを禁止

public:
	/// オブジェクト生成はstaticメンバから。コンストラクタは呼び出せない。
	static MyBitmap *load(IDirect3DDevice9 *dev, TCHAR *font);
	virtual ~MyBitmap() {
		if (tex) tex->Release();
		if (spr) spr->Release();
	}
	float draw(int dx, int dy, int xx, int yy, float stride, int width, int height);
};

ID3DXSprite *MyBitmap::spr;

//オブジェクトを生成し、画像を読み込む。
MyBitmap *MyBitmap::load(IDirect3DDevice9 *dev, TCHAR *fname) {
	MyBitmap *font = new MyBitmap;
	if (font->spr == 0) D3DXCreateSprite(dev, &(font->spr));
	::loadTexture(dev, fname, &(font->tex));
	return font;
}

float MyBitmap::draw(int dx, int dy, int xx, int yy, float stride, int ww, int hh) { //実際に描画を行う関数
	spr->Begin(D3DXSPRITE_ALPHABLEND);
	D3DXVECTOR3 cnt(0, 0, 0);
	D3DXVECTOR3 pos(xx, yy, 0);
	int uu = dx;
	int vv = dy;

	RECT rc = { uu, vv, uu + ww, vv + hh };			// 描画を始める位置
	spr->Draw(tex, &rc, &cnt, &pos, 0xFFFFFFFF);	// 白で
	//xx += stride;
	spr->End();
	return xx;
}

void Title(DataSet *data) { //タイトルシーンに必要な値をdrow関数へ渡す関数

	int cnt = 0;

	while (cnt != 2) {

		int dx = data->drow_x[data->Scene][cnt];			//Bitmapの描画を始める位置のx軸
		int dy = data->drow_y[data->Scene][cnt];			//Bitmapの描画を始める位置のy軸
		int x = data->bit_x[data->Scene][cnt];				//Bitmapを表示するx軸
		int y = data->bit_y[data->Scene][cnt];				//Bitmapを表示するy軸
		int width = data->bit_width[data->Scene][cnt];		//Bitmapの表示する範囲の横の大きさ
		int height = data->bit_height[data->Scene][cnt];	//Bitmapの表示する範囲の横の大きさ

		data->font->draw(dx, dy, x, y, 0, width, height); //描画を始める座標のx軸、y軸,表示するx軸、y軸,字間,大きさ

		cnt++;
	}

}

void Player_Jump(DataSet *data) { //Playerがジャンプするときの値を追加する関数

	data->bit_y[data->Scene][Player] -= 7;
	data->bit_y[data->Scene][Hoeel] -= 7;

	if (data->bit_y[data->Scene][Player] <= data->bit_y[data->Scene][Block] - data->bit_height[data->Scene][Player] - 250) {

		data->Speed++;
		data->jump = false;

	}

}

void PlayerMove(DataSet * data) { //Playerの描画に必要な値を呼び出したdrow関数へ渡す関数

	if (true)
	{
		if (data->jump)Player_Jump(data);

		if (data->bit_y[data->Scene][Player] < data->bit_y[data->Scene][Block] - data->bit_height[data->Scene][Player] - (data->bit_height[data->Scene][Hoeel] / 2)) {

			data->bit_y[data->Scene][Player] += 2;
			data->bit_y[data->Scene][Hoeel] += 2;
			data->Spin = 0;

		}

		if (data->life == 0) {
			//data->bit_y[data->Scene][Player] -= 50;
			data->Scene = RESULT;
		}

		int dx = data->drow_x[data->Scene][Player];			//Bitmapの描画を始める位置のx軸
		int dy = data->drow_y[data->Scene][Player];			//Bitmapの描画を始める位置のy軸
		int x = data->bit_x[data->Scene][Player];			//Bitmapを表示するx軸
		int y = data->bit_y[data->Scene][Player];			//Bitmapを表示するy軸
		int width = data->bit_width[data->Scene][Player];	//Bitmapの表示する範囲の横の大きさ
		int height = data->bit_height[data->Scene][Player];	//Bitmapの表示する範囲の横の大きさ

		data->font->draw(dx, dy, x, y, 0, width, height); //描画を始める座標のx軸、y軸,表示するx軸、y軸,字間,大きさ

		dx = data->drow_x[data->Scene][Hoeel] + data->Spin;			//Bitmapの描画を始める位置のx軸
		dy = data->drow_y[data->Scene][Hoeel];			//Bitmapの描画を始める位置のy軸
		x = data->bit_x[data->Scene][Hoeel];			//Bitmapを表示するx軸
		y = data->bit_y[data->Scene][Hoeel];			//Bitmapを表示するy軸
		width = data->bit_width[data->Scene][Hoeel];	//Bitmapの表示する範囲の横の大きさ
		height = data->bit_height[data->Scene][Hoeel];	//Bitmapの表示する範囲の横の大きさ

		data->font->draw(dx, dy, x, y, 0, width, height); //描画を始める座標のx軸、y軸,表示するx軸、y軸,字間,大きさ

		x += 80;

		data->font->draw(dx, dy, x, y, 0, width, height); //描画を始める座標のx軸、y軸,表示するx軸、y軸,字間,大きさ

		int p = 0;

		for (int i = 0; i != data->life; i++) {

			dx = data->drow_x[data->Scene][Life];			//Bitmapの描画を始める位置のx軸
			dy = data->drow_y[data->Scene][Life];			//Bitmapの描画を始める位置のy軸
			x = data->bit_x[data->Scene][Life];				//Bitmapを表示するx軸
			y = data->bit_y[data->Scene][Life];				//Bitmapを表示するy軸
			width = data->bit_width[data->Scene][Life];		//Bitmapの表示する範囲の横の大きさ
			height = data->bit_height[data->Scene][Life];	//Bitmapの表示する範囲の横の大きさ

			data->font->draw(dx, dy, x - p, y, 0, width, height); //描画を始める座標のx軸、y軸,表示するx軸、y軸,字間,大きさ

			p += 80;

		}

		if (data->Spin == 159) { //一回転したので最初に戻す

			data->Spin = 0;

		}
		else {

			data->Spin += 53;

		}

	}
}

void BlockMove(DataSet *data) { //Blockの描画に必要な値を呼び出したdrow関数へ渡す関数

	int cnt,p = 0;

	bool spown = true;

	if (true)
	{
		while(spown) {

			int dx = data->drow_x[data->Scene][Block];			//Bitmapの描画を始める位置のx軸
			int dy = data->drow_y[data->Scene][Block];			//Bitmapの描画を始める位置のy軸
			int x =  data->Sclrol + p;						    //Bitmapを表示するx軸(スクロールさせるため他の変数を置いている)
			int y =  data->bit_y[data->Scene][Block];			//Bitmapを表示するy軸
			int width = data->bit_width[data->Scene][Block];	//Bitmapの表示する範囲の横の大きさ
			int height = data->bit_height[data->Scene][Block];	//Bitmapの表示する範囲の横の大きさ

			if (0 < x + data->bit_width[data->Scene][Block]) {

				data->font->draw(dx, dy, x, y, 0, width, height); //描画を始める座標のx軸、y軸,表示するx軸、y軸,字間,大きさ

			}

			p += 190;

			if (x >= WIDTH) {

				spown = false;

			}
		}

	}

}

void EnemyMove(DataSet * data) { //Enemyの描画に必要な値を呼び出したdrow関数へ渡す関数

	if (true)
	{
		int dx = data->drow_x[data->Scene][Enemy];			//Bitmapの描画を始める位置のx軸
		int dy = data->drow_y[data->Scene][Enemy];			//Bitmapの描画を始める位置のy軸
		int x = data->Sclrol + data->Range;					//Bitmapを表示するx軸(スクロールに合わせて移動)
		int y = data->bit_y[data->Scene][Enemy];			//Bitmapを表示するy軸
		int width = data->bit_width[data->Scene][Enemy];	//Bitmapの表示する範囲の横の大きさ
		int height = data->bit_height[data->Scene][Enemy];	//Bitmapの表示する範囲の横の大きさ

		if (data->bit_x[data->Scene][Player] + data->bit_width[data->Scene][Player] >= x && data->bit_x[data->Scene][Player] <= x + width &&
			data->bit_y[data->Scene][Player] + data->bit_height[data->Scene][Player] >= y && data->bit_y[data->Scene][Player] <= y + height) {

			data->Range += 1500;

			data->life--;

			data->Speed = 5;
		}

		if (x + height <= 0) data->Range += 1800; //次の座標へ移動する

		data->font->draw(dx, dy, x, y, 0, width, height); //描画を始める座標のx軸、y軸,表示するx軸、y軸,字間,大きさ

	}

}

void Result(DataSet *data) { //リザルトシーンに必要な値を呼び出したdrow関数に代入する関数

	int dx = data->drow_x[data->Scene][0];			//Bitmapの描画を始める位置のx軸
	int dy = data->drow_y[data->Scene][0];			//Bitmapの描画を始める位置のy軸
	int x = data->bit_x[data->Scene][0];			//Bitmapを表示するx軸
	int y = data->bit_y[data->Scene][0];			//Bitmapを表示するy軸
	int width = data->bit_width[data->Scene][0];	//Bitmapの表示する範囲の横の大きさ
	int height = data->bit_height[data->Scene][0];	//Bitmapの表示する範囲の横の大きさ

	data->font->draw(dx, dy, x, y, 0, width, height); //描画を始める座標のx軸、y軸,表示するx軸、y軸,字間,大きさ

}

/// DataSetに基づいて描画を行う
HRESULT drawData(DataSet *data) {

	int front = data->Sclrol + (WIDTH * data->Scroll_Reset);
	int hind = data->Sclrol + (WIDTH * (data->Scroll_Reset + 1));

	if (data->Scene == 1) { //ゲームシーンの時にスクロールする

		data->Sclrol -= data->Speed;

	}

	if (front <= -(WIDTH)) { //画面の外まで移動したので座標を後ろに

		data->Scroll_Reset++;

	}

	HRESULT hr = D3D_OK;

	// 背景色を決める。RGB=(50,50,50)とする。
	D3DCOLOR rgb = D3DCOLOR_XRGB(50, 50, 50);;
	// 画面全体を消去。
	data->dev->Clear(0, NULL, D3DCLEAR_TARGET, rgb, 1.0f, 0);

	// 描画を開始（シーン描画の開始）
	data->dev->BeginScene();

	RECT Rect = {0, 0, WIDTH, HEIGHT};

	D3DXVECTOR3 cnt(0, 0, 0);

	D3DXVECTOR3 pos(front, 0, 0);

	D3DXVECTOR3 pos2(hind, 0, 0);

	// スプライトの描画を開始
	data->spr->Begin(D3DXSPRITE_ALPHABLEND);

	data->spr->Draw(data->Back, &Rect, &cnt, &pos, 0xFFFFFFFF);

	data->spr->Draw(data->Back, &Rect, &cnt, &pos2, 0xFFFFFFFF);

	// スプライトの描画を終了、シーンの描画を終了
	data->spr->End();

	switch (data->Scene)
	{
	case TITLE:

		Title(data);

		break;

	case GAME:

		PlayerMove(data);

		BlockMove(data);

		EnemyMove(data);

		break;

	case RESULT:

		Result(data);

		break;

	default:
		break;
	}

	data->dev->EndScene();

	// 実際に画面に表示。バックバッファからフロントバッファへの転送
	// デバイス生成時のフラグ指定により、ここでVSYNCを待つ。
	data->dev->Present(NULL, NULL, NULL, NULL);

	return D3D_OK;
}

/// イベント処理コールバック（ウィンドウプロシージャ）。
/// イベント発生時にDispatchMessage関数から呼ばれる
/// \param hWnd イベントの発生したウィンドウ
/// \param uMsg イベントの種類を表すID
/// \param wParam 第一メッセージパラメータ
/// \param lParam 第二メッセージパラメータ
/// \return DefWindowProcの戻り値に従う
///
LRESULT CALLBACK
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	// イベントの種類に応じて、switch文にて処理を切り分ける。
	switch (uMsg) {
	case WM_LBUTTONDOWN: {

		mydata.spr_x = LOWORD(lParam);
		mydata.spr_y = HIWORD(lParam);

		switch (mydata.Scene)
		{
		case 0:

			if (mydata.spr_x >= mydata.bit_x[mydata.Scene][1] && mydata.spr_x <= mydata.bit_x[mydata.Scene][1] + mydata.bit_width[mydata.Scene][1] &&
				mydata.spr_y >= mydata.bit_y[mydata.Scene][1] && mydata.spr_y <= mydata.bit_y[mydata.Scene][1] + mydata.bit_height[mydata.Scene][1]) {

				mydata.Scene = 1;

			}

			break;

		case 1:

			if (mydata.spr_x >= mydata.bit_x[mydata.Scene][Player] && mydata.spr_x <= mydata.bit_x[mydata.Scene][Player] + mydata.bit_width[mydata.Scene][Player] &&
				mydata.spr_y >= mydata.bit_y[mydata.Scene][Player] && mydata.spr_y <= mydata.bit_y[mydata.Scene][Player] + mydata.bit_height[mydata.Scene][Player]) {

				mydata.jump = true;

			}

			break;

		case 2:

			mydata.Scene = 0;

			mydata.Scroll_Reset = 0;

			mydata.Spin = 0;

			mydata.Range = 1500;

			mydata.life = 5;

			mydata.Speed = 5;

			mydata.jump = false;

			break;

		default:
			break;
		}

		printf("Click\n");

		break;
	}
	case WM_MOUSEMOVE: {
		mydata.spr_x = LOWORD(lParam);
		mydata.spr_y = HIWORD(lParam);

	}
	case WM_KEYDOWN:
		// ESCキーが押下されたら終了
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
		break;

	case WM_CLOSE:		// 終了通知(CLOSEボタンが押された場合など)が届いた場合
		// プログラムを終了させるため、イベントループに0を通知する。
		// この結果、GetMessageの戻り値は0になる。
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	// デフォルトのウィンドウイベント処理
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/// Windowを作成する。
/// \return ウィンドウハンドル
///
HWND initWindow(DataSet *data) {
	// まずウィンドウクラスを登録する。
	// これはウィンドウ生成後の処理の仕方をWindowsに教えるためである。
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));	// 変数wcをゼロクリアする
	wc.cbSize = sizeof(WNDCLASSEX);			// この構造体の大きさを与える
	wc.lpfnWndProc = (WNDPROC)WindowProc;	// ウィンドウプロシージャ登録
	wc.hInstance = data->hInstance;				// インスタンスハンドルを設定
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);	// マウスカーソルの登録
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);	// 背景をGRAYに
	wc.lpszClassName = _T("directx");	// クラス名、CreateWindowと一致させる
	RegisterClassEx(&wc);			// 登録

	// ウィンドウを作成する。クラス名は"directx"
	data->hWnd = CreateWindow(_T("directx"), "トロッコ・ジャンプ・ドッジ", WS_OVERLAPPEDWINDOW,
		0, 0, WIDTH, HEIGHT, NULL, NULL, data->hInstance, NULL);

	return data->hWnd;
}

/// 画像を読み込む。カレントで失敗したら..から読み込む
/// \param fname ファイル名。const修飾子をつけ関数内で値を変更しないことを宣言する。
/// \param tex 作成するテクスチャへのポインタを入れるためのポインタ。
HRESULT loadTexture(IDirect3DDevice9 *dev, LPCTSTR fname, IDirect3DTexture9 **tex) {
	HRESULT hr = D3DXCreateTextureFromFile(dev, fname, tex);
	if (FAILED(hr)) {
		// 失敗したので一つ上のフォルダからテクスチャを探してみる
		int flen = (int)_tcslen(fname) + 4;
		_tprintf(_T("%s: 親ディレクトリから探索...\n"), fname);
		TCHAR *fn = new TCHAR[flen];
		_stprintf_s(fn, flen, _T("..\\%s"), fname);
		hr = D3DXCreateTextureFromFile(dev, fn, tex);
		delete[] fn;
		if (FAILED(hr)) {
			// 失敗したので例外を発生させて終わる
			MessageBox(NULL, _T("テクスチャ読み込み失敗"), fname, MB_OK);
			throw hr;		// エラーが発生したので例外を送る
		}
	}
	return hr;
}

/// DataSetを解放する。
void ReleaseDataSet(DataSet *data) {
	if (data->font) delete data->font;
	if (data->spr) data->spr->Release();
	if (data->dev) data->dev->Release();
	if (data->pD3D) data->pD3D->Release();
	if (data->Back) data->Back->Release();
}

vector<string> split(string& input, char delimiter)
{
	istringstream stream(input);
	string field;
	vector<string> result;
	while (getline(stream, field, delimiter)) {
		result.push_back(field);
	}
	return result;
}

void MainLoop(DataSet *data) {

	int cnt = 0;

	int s;

	int Object_cnt[5];

	ifstream ifs("data.csv");

	string line;

	int i = 0;

	Object_cnt[TITLE] = 2;
	Object_cnt[GAME] = 5;
	Object_cnt[RESULT] = 1;

	while (getline(ifs, line)) {

		vector<string> strvec = split(line, ',');

		s = stoi(strvec.at(0));

		data->drow_x[i][cnt] = s;
		printf("%5d\n", s);

		s = stoi(strvec.at(1));

		data->drow_y[i][cnt] = s;
		printf("%5d\n", s);

		s = stoi(strvec.at(2));

		data->bit_x[i][cnt] = s;
		printf("%5d\n", s);

		s = stoi(strvec.at(3));

		data->bit_y[i][cnt] = s;
		printf("%5d\n", s);

		s = stoi(strvec.at(4));

		data->bit_width[i][cnt] = s;
		printf("%5d\n", s);

		s = stoi(strvec.at(5));

		data->bit_height[i][cnt] = s;
		printf("%5d\n", s);

		cnt++;

		if (Object_cnt[i] == cnt) {

			i++;
			cnt = 0;

		}

	}


	HRESULT hr = E_FAIL;

	data->hWnd = initWindow(data);			// ウィンドウを作成する

	data->Scene = 0;

	data->Scroll_Reset = 0;

	data->Spin = 0;

	data->Range = 1500;

	data->life = 5;

	data->Speed = 5;

	data->jump = false;

	// Direct3Dを初期化する
	hr = initDirect3D(data);

	hr = D3DXCreateTextureFromFile(data->dev, "Rock.png", &(data->Back));

	if (FAILED(hr)) {
		MessageBox(NULL, d3dErrStr(hr), _T("Direct3D初期化失敗"), MB_OK);
		return;
	}
	// スプライトの作成
	hr = D3DXCreateSprite(data->dev, &(data->spr));
	if (FAILED(hr)) {
		MessageBox(NULL, d3dErrStr(hr), _T("スプライト作成失敗"), MB_OK);
		return;
	}
	try {
		// fontBitmap.pngを指定してビットマップフォントオブジェクトを生成
		data->font = MyBitmap::load(data->dev, _T("Object.png"));
	}
	catch (...) {
		return;
	}
	ShowWindow(data->hWnd, SW_SHOWNORMAL);	// 作成したウィンドウを表示する。

	// イベントメッセージを受け取り、
	// ウィンドウプロシージャ(WindowProc)に送出する。
	MSG msg;
	bool flag = 1;
	while (flag) {
		// メッセージがあるかどうか確認する
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			// メッセージがあるので処理する
			if (GetMessage(&msg, NULL, 0, 0) == 1) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				flag = 0;
			}
		}
		else {
			// 描画ルーチンを呼び出す
			drawData(data);
		}
	}
	ReleaseDataSet(data);
}

/// \param argc コマンドラインから渡された引数の数
/// \param argv 引数の実体へのポインタ配列
/// \return 0	正常終了
int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "Japanese");

	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);

	// このプログラムが実行されるときのインスタンスハンドルを取得
	mydata.hInstance = GetModuleHandle(NULL);
	MainLoop(&mydata);

	_CrtDumpMemoryLeaks();		//メモリリークを検出する

	return 0;
}


/// Direct3Dを初期化する
/// \param data データセット
/// \return 発生したエラーまたはD3D_OK
///
HRESULT initDirect3D(DataSet *data) {
	HRESULT hr;

	// Direct3Dインスタンスオブジェクトを生成する。
	// D3D_SDK_VERSIONと、ランタイムバージョン番号が適切でないと、NULLが返る。
	data->pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	// NULLならランタイムが不適切
	if (data->pD3D == NULL) return E_FAIL;

	// PRESENTパラメータをゼロクリアし、適切に初期化
	ZeroMemory(&(data->d3dpp), sizeof(data->d3dpp));
	// フルスクリーンかウィンドウモードか FULLSCREENは先頭でdefineされている。
	data->d3dpp.Windowed = TRUE;
	data->d3dpp.BackBufferHeight = HEIGHT;
	data->d3dpp.BackBufferWidth = WIDTH;
	data->d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// バックバッファはRGBそれぞれ８ビットで。
	data->d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	// Present時に垂直同期に合わせる
	data->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	// D3Dデバイスオブジェクトの作成。HAL&HARD
	hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, data->hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
	if (FAILED(hr)) {
		// D3Dデバイスオブジェクトの作成。REF&HARD
		hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, data->hWnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
	}
	if (FAILED(hr)) {
		// D3Dデバイスオブジェクトの作成。HAL&SOFT
		hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, data->hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
	}
	if (FAILED(hr)) {
		// D3Dデバイスオブジェクトの作成。REF&SOFT
		hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, data->hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
	}
	// ここまできてFAILなら、D3D環境が不適切すぎる
	if (FAILED(hr)) return hr;

	return D3D_OK;
}

/// エラー発生時のHRESULTを文字列に変換するための補助関数
const LPCTSTR d3dErrStr(HRESULT res) {
	switch (res) {
	case D3D_OK: return _T("D3D_OK");
	case D3DERR_DEVICELOST: return _T("D3DERR_DEVICELOST");
	case D3DERR_DRIVERINTERNALERROR: return _T("D3DERR_DRIVERINTERNALERROR");
	case D3DERR_INVALIDCALL: return _T("D3DERR_INVALIDCALL");
	case D3DERR_OUTOFVIDEOMEMORY: return _T("D3DERR_OUTOFVIDEOMEMORY");
	case D3DERR_DEVICENOTRESET: return _T("D3DERR_DEVICENOTRESET");
	case D3DERR_NOTAVAILABLE: return _T("D3DERR_NOTAVAILABLE");
	case D3DXERR_INVALIDDATA: return _T("D3DXERR_INVALIDDATA");
	case MMSYSERR_NODRIVER: return _T("MMSYSERR_NODRIVER");
	case MMSYSERR_INVALPARAM: return _T("MMSYSERR_INVALPARAM");
	case MMSYSERR_BADDEVICEID: return _T("MMSYSERR_BADDEVICEID");
	case JOYERR_UNPLUGGED: return _T("JOYERR_UNPLUGGED");
	case JOYERR_PARMS: return _T("JOYERR_PARMS");
	case E_OUTOFMEMORY: return _T("E_OUTOFMEMORY");
	}
	return _T("unknown error");
}