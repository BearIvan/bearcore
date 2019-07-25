

 void BearString::ToClipboard(const bchar8 * text)
{
	 ToClipboard(text, GetSize(text));
}

 void BearString::ToClipboard(const bchar16 * text)
{
	 ToClipboard(*BearEncoding::ToANSI(text));
}

 void BearString::ToClipboard(const bchar_utf8 * text)
{
	 ToClipboard(*BearEncoding::ToANSI(text));
}

 void BearCore::BearString::ToClipboard(const bchar8 * text, bsize size)
 {
	 if (OpenClipboard(0))
	 {
		 HGLOBAL hgBuffer;
		 bchar8* chBuffer;
		 EmptyClipboard(); //������� �����
		 hgBuffer = GlobalAlloc(GMEM_DDESHARE, size + 1);//�������� ������
		 chBuffer = (char*)GlobalLock(hgBuffer); //��������� ������
		 CopyWithSizeLimit(chBuffer, size + 1, text, size);
		 GlobalUnlock(hgBuffer);//������������ ������
		 SetClipboardData(CF_TEXT, hgBuffer);//�������� ����� � ����� ������
		 CloseClipboard();
	 }
	
 }

  void BearCore::BearString::ToClipboard(const bchar16 * text, bsize size)
 {
	  ToClipboard(*BearEncoding::ToANSI(text), size);
 }

 void BearCore::BearString::ToClipboard(const bchar_utf8 * text, bsize size)
 {
	 ToClipboard(*BearEncoding::ToANSI(text), size);
 }

BearString BearString::GetClipboard()
{
	 BearString result;
	 if (OpenClipboard(0))
	 {
		 HANDLE hData = GetClipboardData(CF_TEXT);//��������� ����� �� ������ ������
		 bchar8* chBuffer = (bchar8*)GlobalLock(hData);//��������� ������
		 result.assign(*BearEncoding::ToCurrent(chBuffer));
		 GlobalUnlock(hData);//������������ ������
		 CloseClipboard();//��������� ����� ������
	 }
	 return result;
}
