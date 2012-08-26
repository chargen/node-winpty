#include <string>

wchar_t* ToWChar(const char* utf8){
  if (utf8 == NULL || *utf8 == L'\0') {
    return new wchar_t[0];
  } else {
    const int utf8len = static_cast<int>(strlen(utf8));
    const int utf16len = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, utf8len, NULL, 0);
    if (utf16len == 0) {
      return new wchar_t[0];
    } else {
      wchar_t* utf16 = new wchar_t[utf16len];
      if (!::MultiByteToWideChar(CP_UTF8, 0, utf8, utf8len, utf16, utf16len)) {
        return new wchar_t[0];
      } else {
        return utf16;
      }
    }
  }
}

char* ToChar(const wchar_t* utf16){
  if (utf16 == NULL || *utf16 == L'\0') {
    return new char[0];
  } else {
    const int utf16len = static_cast<int>(wcslen(utf16));
    const int utf8len = ::WideCharToMultiByte(CP_UTF8, 0, utf16, utf16len, NULL, 0, NULL, NULL);
    if (utf8len == 0) {
      return new char[0];
    } else {
      char* utf8 = new char[utf8len];
      if (!::WideCharToMultiByte(CP_UTF8, 0, utf16, utf16len, utf8, utf8len, NULL, NULL)) {
        return new char[0];
      } else {
        return utf8;
      }
    }
  }
}


int UnicodeToAnsi(const wchar_t* utf16, size_t sizew, char* utf8, size_t size) {
  return WideCharToMultiByte(CP_UTF8, 0, utf16, sizew, utf8, size, NULL, NULL);
}

int AnsiToUnicode(const char* utf8, wchar_t* utf16, size_t sizew) {
  return MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, sizew);
}


wchar_t* QuoteArg(const wchar_t *source, wchar_t *target) {
  int len = wcslen(source), i, quote_hit;
  wchar_t* start;

  if (len == 0) return target;

  if (NULL == wcspbrk(source, L" \t\"")) {
    wcsncpy(target, source, len);
    target += len;
    return target;
  }

  if (NULL == wcspbrk(source, L"\"\\")) {
    *(target++) = L'"';
    wcsncpy(target, source, len);
    target += len;
    *(target++) = L'"';
    return target;
  }

  *(target++) = L'"';
  start = target;
  quote_hit = 1;

  for (i = len; i > 0; --i) {
    *(target++) = source[i - 1];

    if (quote_hit && source[i - 1] == L'\\') {
      *(target++) = L'\\';
    } else if(source[i - 1] == L'"') {
      quote_hit = 1;
      *(target++) = L'\\';
    } else {
      quote_hit = 0;
    }
  }
  target[0] = L'\0';
  wcsrev(start);
  *(target++) = L'"';
  return target;
}
