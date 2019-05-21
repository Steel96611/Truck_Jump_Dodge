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

// �ȉ��̓v���W�F�N�g�ւ̐ݒ薳���Ń��C�u�����������N���邽�߂̋L�q
#pragma comment(lib, "d3d9.lib")	// Direct3D9 ���C�u�����������N����
#pragma comment(lib, "d3dx9.lib")	// Direct3DX9 ���C�u�����������N����

struct DataSet;
class MyBitmap;

// �֐��錾
HRESULT initDirect3D(DataSet *data);
const LPCTSTR d3dErrStr(HRESULT res);
HRESULT loadTexture(IDirect3DDevice9 *dev, LPCTSTR fname, IDirect3DTexture9 **tex);

// �E�B���h�E�̕��ƍ�����萔�Őݒ肷��
#define WIDTH 1520		//�E�B���h�E�̕�
#define HEIGHT 800		//�E�B���h�E�̍���

// �z��ɏ��Ԃɂ��ꂼ��̃I�u�W�F�N�g�Ɋւ���l��������
#define Player 0		//�v���C���[
#define Hoeel  1		//�z�C�[��
#define Life   2		//���C�t
#define Block  3		//�u���b�N
#define Enemy  4		//�G�l�~�[

#define TITLE  0		//�^�C�g���V�[��
#define GAME   1		//�Q�[���V�[��
#define RESULT 2		//���U���g�V�[��

/// �v���O�����ɕK�v�ȕϐ����\���̂Ƃ��Ē�`
struct DataSet {
	HINSTANCE hInstance;			//�C���X�^���X�n���h��
	HWND hWnd;						//�\���E�B���h�E

	IDirect3D9 *pD3D;				//Direct3D�C���X�^���X�I�u�W�F�N�g
	D3DPRESENT_PARAMETERS d3dpp;	//�f�o�C�X�쐬���̃p�����[�^

	IDirect3DTexture9 *Back;		//�X�v���C�g�ɓ\��t����e�N�X�`��

	IDirect3DDevice9 *dev;			//Direct3D�f�o�C�X�I�u�W�F�N�g
	ID3DXSprite *spr;				//�X�v���C�g

	MyBitmap *font;			//�r�b�g�}�b�v

	int spr_x, spr_y;		//�X�v���C�g�\���ʒu

	int drow_x[10][10];		//�����o���n�߂���W��x��
	int drow_y[10][10];		//�����o���n�߂���W��y��
	int bit_x[10][10];		//�`�悷��ʒu��x��
	int bit_y[10][10];		//�`�悷��ʒu��y��
	int bit_width[10][10];	//�\�����鉡��
	int bit_height[10][10];	//�\������c��

	int Sclrol;				//�w�i�̃X�N���[���̒l

	int Scroll_Reset;		//�X�N���[���̃��Z�b�g

	int Scene;				//�Q�[���̃V�[��

	int Spin;				//Player�̎ԗւ̉�]

	int Range;				//Enemy�̍��W�v�Z�ɕK�v�Ȓl

	int life;				//�c�@

	int Speed;				//�X�N���[���̑��x

	bool jump;				//Player�̏��(���ł��邩�ǂ���)

} mydata;

//�r�b�g�}�b�v�ŕ\�����邽�߂̃N���X
class MyBitmap {
private:
	IDirect3DTexture9 *tex;
	static ID3DXSprite *spr;
	MyBitmap() :tex(0) {}		// �R���X�g���N�^�̌Ăяo�����֎~

public:
	/// �I�u�W�F�N�g������static�����o����B�R���X�g���N�^�͌Ăяo���Ȃ��B
	static MyBitmap *load(IDirect3DDevice9 *dev, TCHAR *font);
	virtual ~MyBitmap() {
		if (tex) tex->Release();
		if (spr) spr->Release();
	}
	float draw(int dx, int dy, int xx, int yy, float stride, int width, int height);
};

ID3DXSprite *MyBitmap::spr;

//�I�u�W�F�N�g�𐶐����A�摜��ǂݍ��ށB
MyBitmap *MyBitmap::load(IDirect3DDevice9 *dev, TCHAR *fname) {
	MyBitmap *font = new MyBitmap;
	if (font->spr == 0) D3DXCreateSprite(dev, &(font->spr));
	::loadTexture(dev, fname, &(font->tex));
	return font;
}

float MyBitmap::draw(int dx, int dy, int xx, int yy, float stride, int ww, int hh) { //���ۂɕ`����s���֐�
	spr->Begin(D3DXSPRITE_ALPHABLEND);
	D3DXVECTOR3 cnt(0, 0, 0);
	D3DXVECTOR3 pos(xx, yy, 0);
	int uu = dx;
	int vv = dy;

	RECT rc = { uu, vv, uu + ww, vv + hh };			// �`����n�߂�ʒu
	spr->Draw(tex, &rc, &cnt, &pos, 0xFFFFFFFF);	// ����
	//xx += stride;
	spr->End();
	return xx;
}

void Title(DataSet *data) { //�^�C�g���V�[���ɕK�v�Ȓl��drow�֐��֓n���֐�

	int cnt = 0;

	while (cnt != 2) {

		int dx = data->drow_x[data->Scene][cnt];			//Bitmap�̕`����n�߂�ʒu��x��
		int dy = data->drow_y[data->Scene][cnt];			//Bitmap�̕`����n�߂�ʒu��y��
		int x = data->bit_x[data->Scene][cnt];				//Bitmap��\������x��
		int y = data->bit_y[data->Scene][cnt];				//Bitmap��\������y��
		int width = data->bit_width[data->Scene][cnt];		//Bitmap�̕\������͈͂̉��̑傫��
		int height = data->bit_height[data->Scene][cnt];	//Bitmap�̕\������͈͂̉��̑傫��

		data->font->draw(dx, dy, x, y, 0, width, height); //�`����n�߂���W��x���Ay��,�\������x���Ay��,����,�傫��

		cnt++;
	}

}

void Player_Jump(DataSet *data) { //Player���W�����v����Ƃ��̒l��ǉ�����֐�

	data->bit_y[data->Scene][Player] -= 7;
	data->bit_y[data->Scene][Hoeel] -= 7;

	if (data->bit_y[data->Scene][Player] <= data->bit_y[data->Scene][Block] - data->bit_height[data->Scene][Player] - 250) {

		data->Speed++;
		data->jump = false;

	}

}

void PlayerMove(DataSet * data) { //Player�̕`��ɕK�v�Ȓl���Ăяo����drow�֐��֓n���֐�

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

		int dx = data->drow_x[data->Scene][Player];			//Bitmap�̕`����n�߂�ʒu��x��
		int dy = data->drow_y[data->Scene][Player];			//Bitmap�̕`����n�߂�ʒu��y��
		int x = data->bit_x[data->Scene][Player];			//Bitmap��\������x��
		int y = data->bit_y[data->Scene][Player];			//Bitmap��\������y��
		int width = data->bit_width[data->Scene][Player];	//Bitmap�̕\������͈͂̉��̑傫��
		int height = data->bit_height[data->Scene][Player];	//Bitmap�̕\������͈͂̉��̑傫��

		data->font->draw(dx, dy, x, y, 0, width, height); //�`����n�߂���W��x���Ay��,�\������x���Ay��,����,�傫��

		dx = data->drow_x[data->Scene][Hoeel] + data->Spin;			//Bitmap�̕`����n�߂�ʒu��x��
		dy = data->drow_y[data->Scene][Hoeel];			//Bitmap�̕`����n�߂�ʒu��y��
		x = data->bit_x[data->Scene][Hoeel];			//Bitmap��\������x��
		y = data->bit_y[data->Scene][Hoeel];			//Bitmap��\������y��
		width = data->bit_width[data->Scene][Hoeel];	//Bitmap�̕\������͈͂̉��̑傫��
		height = data->bit_height[data->Scene][Hoeel];	//Bitmap�̕\������͈͂̉��̑傫��

		data->font->draw(dx, dy, x, y, 0, width, height); //�`����n�߂���W��x���Ay��,�\������x���Ay��,����,�傫��

		x += 80;

		data->font->draw(dx, dy, x, y, 0, width, height); //�`����n�߂���W��x���Ay��,�\������x���Ay��,����,�傫��

		int p = 0;

		for (int i = 0; i != data->life; i++) {

			dx = data->drow_x[data->Scene][Life];			//Bitmap�̕`����n�߂�ʒu��x��
			dy = data->drow_y[data->Scene][Life];			//Bitmap�̕`����n�߂�ʒu��y��
			x = data->bit_x[data->Scene][Life];				//Bitmap��\������x��
			y = data->bit_y[data->Scene][Life];				//Bitmap��\������y��
			width = data->bit_width[data->Scene][Life];		//Bitmap�̕\������͈͂̉��̑傫��
			height = data->bit_height[data->Scene][Life];	//Bitmap�̕\������͈͂̉��̑傫��

			data->font->draw(dx, dy, x - p, y, 0, width, height); //�`����n�߂���W��x���Ay��,�\������x���Ay��,����,�傫��

			p += 80;

		}

		if (data->Spin == 159) { //���]�����̂ōŏ��ɖ߂�

			data->Spin = 0;

		}
		else {

			data->Spin += 53;

		}

	}
}

void BlockMove(DataSet *data) { //Block�̕`��ɕK�v�Ȓl���Ăяo����drow�֐��֓n���֐�

	int cnt,p = 0;

	bool spown = true;

	if (true)
	{
		while(spown) {

			int dx = data->drow_x[data->Scene][Block];			//Bitmap�̕`����n�߂�ʒu��x��
			int dy = data->drow_y[data->Scene][Block];			//Bitmap�̕`����n�߂�ʒu��y��
			int x =  data->Sclrol + p;						    //Bitmap��\������x��(�X�N���[�������邽�ߑ��̕ϐ���u���Ă���)
			int y =  data->bit_y[data->Scene][Block];			//Bitmap��\������y��
			int width = data->bit_width[data->Scene][Block];	//Bitmap�̕\������͈͂̉��̑傫��
			int height = data->bit_height[data->Scene][Block];	//Bitmap�̕\������͈͂̉��̑傫��

			if (0 < x + data->bit_width[data->Scene][Block]) {

				data->font->draw(dx, dy, x, y, 0, width, height); //�`����n�߂���W��x���Ay��,�\������x���Ay��,����,�傫��

			}

			p += 190;

			if (x >= WIDTH) {

				spown = false;

			}
		}

	}

}

void EnemyMove(DataSet * data) { //Enemy�̕`��ɕK�v�Ȓl���Ăяo����drow�֐��֓n���֐�

	if (true)
	{
		int dx = data->drow_x[data->Scene][Enemy];			//Bitmap�̕`����n�߂�ʒu��x��
		int dy = data->drow_y[data->Scene][Enemy];			//Bitmap�̕`����n�߂�ʒu��y��
		int x = data->Sclrol + data->Range;					//Bitmap��\������x��(�X�N���[���ɍ��킹�Ĉړ�)
		int y = data->bit_y[data->Scene][Enemy];			//Bitmap��\������y��
		int width = data->bit_width[data->Scene][Enemy];	//Bitmap�̕\������͈͂̉��̑傫��
		int height = data->bit_height[data->Scene][Enemy];	//Bitmap�̕\������͈͂̉��̑傫��

		if (data->bit_x[data->Scene][Player] + data->bit_width[data->Scene][Player] >= x && data->bit_x[data->Scene][Player] <= x + width &&
			data->bit_y[data->Scene][Player] + data->bit_height[data->Scene][Player] >= y && data->bit_y[data->Scene][Player] <= y + height) {

			data->Range += 1500;

			data->life--;

			data->Speed = 5;
		}

		if (x + height <= 0) data->Range += 1800; //���̍��W�ֈړ�����

		data->font->draw(dx, dy, x, y, 0, width, height); //�`����n�߂���W��x���Ay��,�\������x���Ay��,����,�傫��

	}

}

void Result(DataSet *data) { //���U���g�V�[���ɕK�v�Ȓl���Ăяo����drow�֐��ɑ������֐�

	int dx = data->drow_x[data->Scene][0];			//Bitmap�̕`����n�߂�ʒu��x��
	int dy = data->drow_y[data->Scene][0];			//Bitmap�̕`����n�߂�ʒu��y��
	int x = data->bit_x[data->Scene][0];			//Bitmap��\������x��
	int y = data->bit_y[data->Scene][0];			//Bitmap��\������y��
	int width = data->bit_width[data->Scene][0];	//Bitmap�̕\������͈͂̉��̑傫��
	int height = data->bit_height[data->Scene][0];	//Bitmap�̕\������͈͂̉��̑傫��

	data->font->draw(dx, dy, x, y, 0, width, height); //�`����n�߂���W��x���Ay��,�\������x���Ay��,����,�傫��

}

/// DataSet�Ɋ�Â��ĕ`����s��
HRESULT drawData(DataSet *data) {

	int front = data->Sclrol + (WIDTH * data->Scroll_Reset);
	int hind = data->Sclrol + (WIDTH * (data->Scroll_Reset + 1));

	if (data->Scene == 1) { //�Q�[���V�[���̎��ɃX�N���[������

		data->Sclrol -= data->Speed;

	}

	if (front <= -(WIDTH)) { //��ʂ̊O�܂ňړ������̂ō��W������

		data->Scroll_Reset++;

	}

	HRESULT hr = D3D_OK;

	// �w�i�F�����߂�BRGB=(50,50,50)�Ƃ���B
	D3DCOLOR rgb = D3DCOLOR_XRGB(50, 50, 50);;
	// ��ʑS�̂������B
	data->dev->Clear(0, NULL, D3DCLEAR_TARGET, rgb, 1.0f, 0);

	// �`����J�n�i�V�[���`��̊J�n�j
	data->dev->BeginScene();

	RECT Rect = {0, 0, WIDTH, HEIGHT};

	D3DXVECTOR3 cnt(0, 0, 0);

	D3DXVECTOR3 pos(front, 0, 0);

	D3DXVECTOR3 pos2(hind, 0, 0);

	// �X�v���C�g�̕`����J�n
	data->spr->Begin(D3DXSPRITE_ALPHABLEND);

	data->spr->Draw(data->Back, &Rect, &cnt, &pos, 0xFFFFFFFF);

	data->spr->Draw(data->Back, &Rect, &cnt, &pos2, 0xFFFFFFFF);

	// �X�v���C�g�̕`����I���A�V�[���̕`����I��
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

	// ���ۂɉ�ʂɕ\���B�o�b�N�o�b�t�@����t�����g�o�b�t�@�ւ̓]��
	// �f�o�C�X�������̃t���O�w��ɂ��A������VSYNC��҂B
	data->dev->Present(NULL, NULL, NULL, NULL);

	return D3D_OK;
}

/// �C�x���g�����R�[���o�b�N�i�E�B���h�E�v���V�[�W���j�B
/// �C�x���g��������DispatchMessage�֐�����Ă΂��
/// \param hWnd �C�x���g�̔��������E�B���h�E
/// \param uMsg �C�x���g�̎�ނ�\��ID
/// \param wParam ��ꃁ�b�Z�[�W�p�����[�^
/// \param lParam ��񃁃b�Z�[�W�p�����[�^
/// \return DefWindowProc�̖߂�l�ɏ]��
///
LRESULT CALLBACK
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	// �C�x���g�̎�ނɉ����āAswitch���ɂď�����؂蕪����B
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
		// ESC�L�[���������ꂽ��I��
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
		break;

	case WM_CLOSE:		// �I���ʒm(CLOSE�{�^���������ꂽ�ꍇ�Ȃ�)���͂����ꍇ
		// �v���O�������I�������邽�߁A�C�x���g���[�v��0��ʒm����B
		// ���̌��ʁAGetMessage�̖߂�l��0�ɂȂ�B
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	// �f�t�H���g�̃E�B���h�E�C�x���g����
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/// Window���쐬����B
/// \return �E�B���h�E�n���h��
///
HWND initWindow(DataSet *data) {
	// �܂��E�B���h�E�N���X��o�^����B
	// ����̓E�B���h�E������̏����̎d����Windows�ɋ����邽�߂ł���B
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));	// �ϐ�wc���[���N���A����
	wc.cbSize = sizeof(WNDCLASSEX);			// ���̍\���̂̑傫����^����
	wc.lpfnWndProc = (WNDPROC)WindowProc;	// �E�B���h�E�v���V�[�W���o�^
	wc.hInstance = data->hInstance;				// �C���X�^���X�n���h����ݒ�
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);	// �}�E�X�J�[�\���̓o�^
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);	// �w�i��GRAY��
	wc.lpszClassName = _T("directx");	// �N���X���ACreateWindow�ƈ�v������
	RegisterClassEx(&wc);			// �o�^

	// �E�B���h�E���쐬����B�N���X����"directx"
	data->hWnd = CreateWindow(_T("directx"), "�g���b�R�E�W�����v�E�h�b�W", WS_OVERLAPPEDWINDOW,
		0, 0, WIDTH, HEIGHT, NULL, NULL, data->hInstance, NULL);

	return data->hWnd;
}

/// �摜��ǂݍ��ށB�J�����g�Ŏ��s������..����ǂݍ���
/// \param fname �t�@�C�����Bconst�C���q�����֐����Œl��ύX���Ȃ����Ƃ�錾����B
/// \param tex �쐬����e�N�X�`���ւ̃|�C���^�����邽�߂̃|�C���^�B
HRESULT loadTexture(IDirect3DDevice9 *dev, LPCTSTR fname, IDirect3DTexture9 **tex) {
	HRESULT hr = D3DXCreateTextureFromFile(dev, fname, tex);
	if (FAILED(hr)) {
		// ���s�����̂ň��̃t�H���_����e�N�X�`����T���Ă݂�
		int flen = (int)_tcslen(fname) + 4;
		_tprintf(_T("%s: �e�f�B���N�g������T��...\n"), fname);
		TCHAR *fn = new TCHAR[flen];
		_stprintf_s(fn, flen, _T("..\\%s"), fname);
		hr = D3DXCreateTextureFromFile(dev, fn, tex);
		delete[] fn;
		if (FAILED(hr)) {
			// ���s�����̂ŗ�O�𔭐������ďI���
			MessageBox(NULL, _T("�e�N�X�`���ǂݍ��ݎ��s"), fname, MB_OK);
			throw hr;		// �G���[�����������̂ŗ�O�𑗂�
		}
	}
	return hr;
}

/// DataSet���������B
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

	data->hWnd = initWindow(data);			// �E�B���h�E���쐬����

	data->Scene = 0;

	data->Scroll_Reset = 0;

	data->Spin = 0;

	data->Range = 1500;

	data->life = 5;

	data->Speed = 5;

	data->jump = false;

	// Direct3D������������
	hr = initDirect3D(data);

	hr = D3DXCreateTextureFromFile(data->dev, "Rock.png", &(data->Back));

	if (FAILED(hr)) {
		MessageBox(NULL, d3dErrStr(hr), _T("Direct3D���������s"), MB_OK);
		return;
	}
	// �X�v���C�g�̍쐬
	hr = D3DXCreateSprite(data->dev, &(data->spr));
	if (FAILED(hr)) {
		MessageBox(NULL, d3dErrStr(hr), _T("�X�v���C�g�쐬���s"), MB_OK);
		return;
	}
	try {
		// fontBitmap.png���w�肵�ăr�b�g�}�b�v�t�H���g�I�u�W�F�N�g�𐶐�
		data->font = MyBitmap::load(data->dev, _T("Object.png"));
	}
	catch (...) {
		return;
	}
	ShowWindow(data->hWnd, SW_SHOWNORMAL);	// �쐬�����E�B���h�E��\������B

	// �C�x���g���b�Z�[�W���󂯎��A
	// �E�B���h�E�v���V�[�W��(WindowProc)�ɑ��o����B
	MSG msg;
	bool flag = 1;
	while (flag) {
		// ���b�Z�[�W�����邩�ǂ����m�F����
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			// ���b�Z�[�W������̂ŏ�������
			if (GetMessage(&msg, NULL, 0, 0) == 1) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				flag = 0;
			}
		}
		else {
			// �`�惋�[�`�����Ăяo��
			drawData(data);
		}
	}
	ReleaseDataSet(data);
}

/// \param argc �R�}���h���C������n���ꂽ�����̐�
/// \param argv �����̎��̂ւ̃|�C���^�z��
/// \return 0	����I��
int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "Japanese");

	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);

	// ���̃v���O���������s�����Ƃ��̃C���X�^���X�n���h�����擾
	mydata.hInstance = GetModuleHandle(NULL);
	MainLoop(&mydata);

	_CrtDumpMemoryLeaks();		//���������[�N�����o����

	return 0;
}


/// Direct3D������������
/// \param data �f�[�^�Z�b�g
/// \return ���������G���[�܂���D3D_OK
///
HRESULT initDirect3D(DataSet *data) {
	HRESULT hr;

	// Direct3D�C���X�^���X�I�u�W�F�N�g�𐶐�����B
	// D3D_SDK_VERSION�ƁA�����^�C���o�[�W�����ԍ����K�؂łȂ��ƁANULL���Ԃ�B
	data->pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	// NULL�Ȃ烉���^�C�����s�K��
	if (data->pD3D == NULL) return E_FAIL;

	// PRESENT�p�����[�^���[���N���A���A�K�؂ɏ�����
	ZeroMemory(&(data->d3dpp), sizeof(data->d3dpp));
	// �t���X�N���[�����E�B���h�E���[�h�� FULLSCREEN�͐擪��define����Ă���B
	data->d3dpp.Windowed = TRUE;
	data->d3dpp.BackBufferHeight = HEIGHT;
	data->d3dpp.BackBufferWidth = WIDTH;
	data->d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// �o�b�N�o�b�t�@��RGB���ꂼ��W�r�b�g�ŁB
	data->d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	// Present���ɐ��������ɍ��킹��
	data->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	// D3D�f�o�C�X�I�u�W�F�N�g�̍쐬�BHAL&HARD
	hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, data->hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
	if (FAILED(hr)) {
		// D3D�f�o�C�X�I�u�W�F�N�g�̍쐬�BREF&HARD
		hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, data->hWnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
	}
	if (FAILED(hr)) {
		// D3D�f�o�C�X�I�u�W�F�N�g�̍쐬�BHAL&SOFT
		hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, data->hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
	}
	if (FAILED(hr)) {
		// D3D�f�o�C�X�I�u�W�F�N�g�̍쐬�BREF&SOFT
		hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, data->hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
	}
	// �����܂ł���FAIL�Ȃ�AD3D�����s�K�؂�����
	if (FAILED(hr)) return hr;

	return D3D_OK;
}

/// �G���[��������HRESULT�𕶎���ɕϊ����邽�߂̕⏕�֐�
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