#include "Common.h"
#include <iomanip>

const TCHAR * Extension::Error()
{
	return Runtime.CopyString(!threadData->error.text.empty() ? UTF8ToTString(threadData->error.text).c_str() : _T(""));
}
unsigned int Extension::Channel_Count()
{
	return Srv.channelcount();
}
const TCHAR * Extension::Client_Name()
{
	return Runtime.CopyString(!selClient ? _T("") : UTF8ToTString(selClient->name()).c_str());
}
const TCHAR * Extension::RecvMsg_ReadAsString()
{
	if (threadData->receivedMsg.variant != 0)
		return CreateError("Received$() was used on a message that is not a text message."), Runtime.CopyString(_T(""));

	// UTF-8 validation happens before the Fusion server message handler is called.
	
	// RecvMsg_Sub_ReadString expects size in code points or a null terminator,
	// but in a text message neither is present, so we'll just directly convert.
	return Runtime.CopyString(UTF8ToTString(threadData->receivedMsg.content).c_str());
}
int Extension::RecvMsg_ReadAsInteger()
{
	if (threadData->receivedMsg.variant != 1)
		return CreateError("Received() was used on a message that is not a number message."), 0;

	return *(int *)threadData->receivedMsg.content.data();
}
unsigned int Extension::RecvMsg_Subchannel()
{
	return (int)threadData->receivedMsg.subchannel;
}
unsigned int Extension::Client_ID()
{
	return selClient ? selClient->id() : -1;
}
const TCHAR * Extension::RequestedClientName()
{
	return Runtime.CopyString(UTF8ToTString(threadData->requested.name).c_str());
}
const TCHAR * Extension::RequestedChannelName()
{
	// 4 = Leave Channel condition ID. There's no point rewriting the name in a Leave Channel request.
	if (threadData->CondTrig[0] == 4)
		CreateError("Requested channel name is not available in a Leave Channel Request. Use Channel_Name$() instead.");

	return Runtime.CopyString(UTF8ToTString(threadData->requested.name).c_str());
}
unsigned int Extension::Client_ConnectionTime()
{
	return selClient ? (int)selClient->getconnecttime() : -1;
}
unsigned int Extension::Client_ChannelCount()
{
	return selClient ? selClient->channelcount() : -1;
}
const TCHAR * Extension::Client_GetLocalData(const TCHAR * key)
{
	if (!key || !selClient)
		return Runtime.CopyString(_T(""));
	return Runtime.CopyString(globals->GetLocalData(selClient, key).c_str());
}
const TCHAR * Extension::Channel_GetLocalData(const TCHAR * key)
{
	if (!key || !selChannel)
		return Runtime.CopyString(_T(""));
	return Runtime.CopyString(globals->GetLocalData(selChannel, key).c_str());
}
const TCHAR * Extension::Channel_Name()
{
	return Runtime.CopyString(selChannel ? UTF8ToTString(selChannel->name()).c_str() : _T(""));
}
unsigned int Extension::Channel_ClientCount()
{
	return selChannel ? selChannel->clientcount() : -1;
}
const TCHAR * Extension::RecvMsg_StrASCIIByte(int index)
{
	if (index < 0)
	{
		CreateError("Could not read ASCII text byte from received binary, index %i is less than 0.", index);
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - index < sizeof(char))
	{
		CreateError("Could not read ASCII text byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index);
		return Runtime.CopyString(_T(""));
	}

	const std::string_view partial(threadData->receivedMsg.content.data() + index, 1);
	const std::uint8_t charUnsigned = *(std::uint8_t *)(&partial[0]);

	// ASCII goes up to 127, ANSI continues to 255. ANSI is locale-dependent, and if we only
	// interpret it as a fixed one like "en-us", that may not be the sender's locale, muddying
	// the water further for a Fusion developer.
	if (charUnsigned > 127)
	{
		CreateError("ANSI char %hhu read at position %d, which is not an ASCII character. Use StringWithSize() with size 1 to ensure consistency.",
			charUnsigned, index);
		return Runtime.CopyString(_T(""));
	}
	// Also check character is displayable (aka "printable").
	if (!isprint(charUnsigned))
	{
		CreateError("ASCII char %hhu read, which cannot be displayed. Check index %d is correct.",
			charUnsigned, index);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(ANSIToTString(partial).c_str());
}
unsigned int Extension::RecvMsg_UnsignedByte(int index)
{
	if (index < 0)
		return CreateError("Could not read unsigned byte from received binary, index %i is less than 0.", index), 0U;
	if (threadData->receivedMsg.content.size() - index < sizeof(unsigned char))
		return CreateError("Could not read unsigned byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0U;

	return (unsigned int)(*(unsigned char *)(threadData->receivedMsg.content.data() + index));
}
int Extension::RecvMsg_SignedByte(int index)
{
	if (index < 0)
		return CreateError("Could not read signed byte from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(char))
		return CreateError("Could not read signed byte from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (int)(*(threadData->receivedMsg.content.data() + index));
}
unsigned int Extension::RecvMsg_UnsignedShort(int index)
{
	if (index < 0)
		return CreateError("Could not read unsigned short from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(unsigned short))
		return CreateError("Could not read unsigned short from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (unsigned int)(*(unsigned short *)(threadData->receivedMsg.content.data() + index));
}
int Extension::RecvMsg_SignedShort(int index)
{
	if (index < 0)
		return CreateError("Could not read signed short from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(short))
		return CreateError("Could not read signed short from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (int)(*(short *)(threadData->receivedMsg.content.data() + index));
}
unsigned int Extension::RecvMsg_UnsignedInteger(int index)
{
	if (index < 0)
		return CreateError("Could not read unsigned integer from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(unsigned int))
		return CreateError("Could not read unsigned integer from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (*(unsigned int *)(threadData->receivedMsg.content.data() + index));
}
int Extension::RecvMsg_SignedInteger(int index)
{
	if (index < 0)
		return CreateError("Could not read signed integer from received binary, index %i is less than 0.", index), 0;
	if (threadData->receivedMsg.content.size() - index < sizeof(int))
		return CreateError("Could not read signed integer from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0;

	return (*(int *)(threadData->receivedMsg.content.data() + index));
}
float Extension::RecvMsg_Float(int index)
{
	if (index < 0)
		return CreateError("Could not read float from received binary, index %i is less than 0.", index), 0.0f;
	if (threadData->receivedMsg.content.size() - index < sizeof(float))
		return CreateError("Could not read float from received binary at position %i, amount of message remaining is smaller than variable to be read.", index), 0.0f;

	return (*(float *)(threadData->receivedMsg.content.data() + index));
}
const TCHAR * Extension::RecvMsg_StringWithSize(int index, int size)
{
	if (index < 0)
	{
		CreateError("Could not read string with size from received binary, index %i is less than 0.", index);
		return Runtime.CopyString(_T(""));
	}
	if (size < 0)
	{
		CreateError("Could not read string with size from received binary, supplied size %i is less than 0.", size);
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - index < (size_t)size)
	{
		CreateError("Could not read string with size from received binary at position %i, amount of message remaining is smaller than supplied string size %i.", index, size);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(index, size, false).c_str());
}
const TCHAR * Extension::RecvMsg_String(int index)
{
	if (index < 0)
	{
		CreateError("Could not read null-terminated string from received binary, index %i is less than 0.", index);
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - index < 1)
	{
		CreateError("Could not read null-terminated string from received binary at position %i, amount of message remaining is smaller than variable to be read.", index);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(index, -1, false).c_str());
}
unsigned int Extension::RecvMsg_SizeInBytes()
{
	return threadData->receivedMsg.content.size();
}
const TCHAR * Extension::Lacewing_Version()
{
	static TCHAR version[128] = {};
	if (version[0] == _T('\0'))
	{
		std::tstringstream str;
		str << lw_version() << _T(" / Bluewing ");
#ifdef _UNICODE
		str << _T("Unicode ");
#endif
		str << _T("reimpl b") << lacewing::relayserver::buildnum;
		_tcscpy_s(version, str.str().c_str());
	}
	return Runtime.CopyString(version);
}
unsigned int Extension::SendBinaryMsg_Size()
{
	return SendMsgSize;
}
const TCHAR * Extension::Client_IP()
{
	if (selClient == nullptr)
	{
		CreateError("Could not read client IP, no client selected.");
		return Runtime.CopyString(_T(""));
	}

	char addr[64];
	lw_addr_prettystring(selClient->getaddress().data(), addr, 64);
	return Runtime.CopyString(ANSIToTString(addr).c_str());
}
unsigned int Extension::Port()
{
	return Srv.port();
}
const TCHAR * Extension::Welcome_Message()
{
	return Runtime.CopyString(UTF8ToTString(Srv.getwelcomemessage()).c_str());
}
unsigned int Extension::RecvMsg_MemoryAddress()
{
	return (unsigned int)(long)threadData->receivedMsg.content.data();
}
const TCHAR * Extension::RecvMsg_Cursor_StrASCIIByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(char))
	{
		CreateError("Could not read ASCII byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor);
		return Runtime.CopyString(_T(""));
	}

	const std::string_view partial(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor, 1);
	const std::uint8_t charUnsigned = *(std::uint8_t *)(&partial[0]);
	++threadData->receivedMsg.cursor;

	// ASCII goes up to 127, ANSI continues to 255. ANSI is locale-dependent, and if we only
	// interpret it as a fixed one like "en-us", that may not be the sender's locale, muddying
	// the water further for a Fusion developer.
	if (charUnsigned > 127)
	{
		CreateError("ANSI char %hhu read from cursor position %u, which is not an ASCII character. Use CursorStringWithSize() with size 1 to ensure consistency.",
			charUnsigned, threadData->receivedMsg.cursor - 1);
		return Runtime.CopyString(_T(""));
	}
	// Also check character is displayable (aka "printable").
	if (!isprint(charUnsigned))
	{
		CreateError("ASCII char %hhu read, which cannot be displayed. Check cursor index %u is correct.",
			charUnsigned, threadData->receivedMsg.cursor - 1);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(ANSIToTString(partial).c_str());
}
unsigned int Extension::RecvMsg_Cursor_UnsignedByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned char))
		return CreateError("Could not read unsigned byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0U;

	++threadData->receivedMsg.cursor;
	return (unsigned int)(*(unsigned char *)(threadData->receivedMsg.content.data() + (threadData->receivedMsg.cursor - sizeof(unsigned char))));
}
int Extension::RecvMsg_Cursor_SignedByte()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(char))
		return CreateError("Could not read signed byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0;

	++threadData->receivedMsg.cursor;
	return (int)(*(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(char)));
}
unsigned int Extension::RecvMsg_Cursor_UnsignedShort()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned short))
		return CreateError("Could not read unsigned short from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0U;

	threadData->receivedMsg.cursor += sizeof(unsigned short);
	return (unsigned int)(*(unsigned short *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(unsigned short)));
}
int Extension::RecvMsg_Cursor_SignedShort()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(short))
		CreateError("Could not read signed short from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0;

	threadData->receivedMsg.cursor += sizeof(short);
	return (int)(*(short *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(short)));
}
unsigned int Extension::RecvMsg_Cursor_UnsignedInteger()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(unsigned int))
		return CreateError("Could not read unsigned integer from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0U;

	threadData->receivedMsg.cursor += sizeof(unsigned int);
	return (*(unsigned int *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(unsigned int)));
}
int Extension::RecvMsg_Cursor_SignedInteger()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(int))
		return CreateError("Could not read signed integer from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0;

	threadData->receivedMsg.cursor += sizeof(int);
	return (*(int *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(int)));
}
float Extension::RecvMsg_Cursor_Float()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < sizeof(float))
		return CreateError("Could not read unsigned byte from received binary at cursor position %u, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor), 0.0f;

	threadData->receivedMsg.cursor += sizeof(float);
	return (*(float *)(threadData->receivedMsg.content.data() + threadData->receivedMsg.cursor - sizeof(float)));
}
const TCHAR * Extension::RecvMsg_Cursor_StringWithSize(int size)
{
	if (size < 0)
	{
		CreateError("Could not read string with size from received binary at cursor position %u, supplied size %i is less than 0.", threadData->receivedMsg.cursor, size);
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < (size_t)size)
	{
		CreateError("Could not read string with size from received binary at cursor position %u, amount of message remaining is less than supplied size %i.", threadData->receivedMsg.cursor, size);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(threadData->receivedMsg.cursor, size, true).c_str());
}
const TCHAR * Extension::RecvMsg_Cursor_String()
{
	if (threadData->receivedMsg.content.size() - threadData->receivedMsg.cursor < 1)
	{
		CreateError("Could not read null-terminated string from received binary at cursor position %zu, amount of message remaining is smaller than variable to be read.", threadData->receivedMsg.cursor);
		return Runtime.CopyString(_T(""));
	}

	return Runtime.CopyString(RecvMsg_Sub_ReadString(threadData->receivedMsg.cursor, -1, true).c_str());
}
const TCHAR * Extension::Client_ProtocolImplementation()
{
	if (selClient == nullptr)
	{
		CreateError("Could not read client protocol implementation, no client selected.");
		return Runtime.CopyString(_T(""));
	}
	return Runtime.CopyString(UTF8ToTString(selClient->getimplementation()).c_str());
}
unsigned int Extension::SendBinaryMsg_MemoryAddress()
{
	return (unsigned int)(long)SendMsg;
}
const TCHAR * Extension::RecvMsg_DumpToString(int index, const TCHAR * formatTStr)
{
	if (!formatTStr || formatTStr[0] == _T('\0'))
	{
		CreateError("Dumping message failed; format supplied was null or \"\".");
		return Runtime.CopyString(_T(""));
	}
	if (threadData->receivedMsg.content.size() - index <= 0)
	{
		CreateError("Dumping message failed; index %i is beyond message end index %zu.", index, threadData->receivedMsg.content.size() - 1);
		return Runtime.CopyString(_T(""));
	}

	bool allValid;
	const std::string formatANSI = TStringToANSI(formatTStr, &allValid);
	if (!allValid)
	{
		CreateError("Dumping message failed; format supplied \"%s\" had unrecognised characters. See help file.", TStringToUTF8(formatTStr).c_str());
		return Runtime.CopyString(_T(""));
	}
	const char * format = formatANSI.c_str();


	std::stringstream output;
	output << std::setfill('0') << std::uppercase;
	size_t sizeOfFormat = strlen(format);
	bool varSigned;
	size_t varCount = 0;
	const char * msg = &threadData->receivedMsg.content[index];
	// Example, signed char x10: "+c10"
	for (const char * i = format; i < format + sizeOfFormat;)
	{
		// Skip last iteration's count to get to next variable type letter
		while (isdigit(i[0]))
			++i;

		// We skipped past digits into the end of string
		if (i == format + sizeOfFormat)
			break;

		// Determine if variable should be signed or unsigned
		if (i[0] != '+')
			varSigned = true;
		else
		{
			varSigned = false;
			++i;
		}

		// Count number of expected variables
		varCount = max(atoi(i+1),1);

		// Char
		if (i[0] == 'c')
		{
			++i;
			if (threadData->receivedMsg.content.size()-index < varCount)
			{
				CreateError("Could not dump; message was not large enough to contain all variables.");
				return Runtime.CopyString(_T(""));
			}

			std::uint32_t curChar;
			std::streamsize width = output.width();
			if (varSigned)
			{
				for (unsigned int j = 0; j < varCount; ++j)
				{
					curChar = *(std::uint8_t *)&msg[j];
					output << "Signed char: "sv;
					if (isprint(curChar))
						output << '\'' << *(char *)&curChar << '\'';
					else
						output << "(?)"sv;
					output << " ("sv << curChar << ", 0x"sv << std::hex << std::setw(2) << curChar << std::dec << std::setw(width) << ")\r\n"sv;
				}
			}
			else
			{
				for (unsigned int j = 0; j < varCount; ++j)
				{
					curChar = *(std::uint8_t *)&msg[j];
					output << "Unsigned char: "sv << curChar << " (0x" << std::hex << std::setw(2) << curChar << std::dec << std::setw(width) << ")\r\n"sv;
				}
			}
			output.width(width);
			msg += varCount;

			continue;
		}

		// Short
		if (i[0] == 'h')
		{
			++i;
			if (threadData->receivedMsg.content.size()-index < varCount*sizeof(short))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (varSigned)
			{
				for (unsigned int j = 0; j < varCount; ++j)
					output << "Signed short: "sv << (int)((short *)msg)[j] << "\r\n"sv;
			}
			else
			{
				for (unsigned int j = 0; j < varCount; ++j)
					output << "Unsigned short: "sv << (int)((unsigned short *)msg)[j] << "\r\n"sv;
			}
			msg += varCount*sizeof(short);
			continue;
		}

		// String (null-terminated)
		if (i[0] == 's')
		{
			++i;
			if (varSigned == false)
				CreateError("'+' flag not expected next to 's'; strings cannot be unsigned.");
			for (unsigned int j = 0; j < varCount; ++j)
			{
				size_t u8StrSize = strnlen(msg, threadData->receivedMsg.content.size() - index + 1);
				if (u8StrSize == threadData->receivedMsg.content.size()-index+1)
				{
					CreateError("Could not dump; message was not large enough to contain variables.");
					return Runtime.CopyString(_T(""));
				}

				if (!lw_u8str_validate(std::string_view(msg, u8StrSize)))
				{
					CreateError("Could not dump; the null-terminated string starting at message index %i, read as %i chars long, was not valid UTF-8 text.", index, u8StrSize);
					return Runtime.CopyString(_T(""));
				}

				output << "String: "sv << msg << "\r\n"sv;
				msg += u8StrSize + 1;
			}
			continue;
		}

		// Integer
		if (i[0] == 'i')
		{
			++i;
			if (threadData->receivedMsg.content.size()-index < varCount*sizeof(int))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (varSigned)
			{
				for (unsigned int j = 0; j < varCount; ++j)
					output << "Signed integer: "sv << ((int *)msg)[j] << "\r\n"sv;
			}
			else
			{
				for (unsigned int j = 0; j < varCount; ++j)
					output << "Unsigned integer: "sv << ((unsigned int *)msg)[j] << "\r\n"sv;
			}
			msg += varCount*sizeof(int);
			continue;
		}

		// Floating-point
		if (i[0] == 'f')
		{
			++i;
			if (threadData->receivedMsg.content.size()-index < varCount*sizeof(float))
			{
				CreateError("Could not dump; message was not large enough to contain variables.");
				return Runtime.CopyString(_T(""));
			}
			if (!varSigned)
				CreateError("'+' flag not expected next to 'f'; floats cannot be unsigned.");
			else
			{
				for (unsigned int j = 0; j < varCount; ++j)
					output << "Float: "sv << ((float *)msg)[j] << "\r\n"sv;
			}
			msg += varCount*sizeof(float);
			continue;
		}

		// Did not find identifier; error out
		CreateError("Unrecognised character in dump format: '%hc'. Valid : c, h, s, i, f; operator +.", i[0]);
		return Runtime.CopyString(_T(""));
	}

	std::string outputTrim = output.str();
	outputTrim.resize(outputTrim.size() - 2);
	return Runtime.CopyString(UTF8ToTString(outputTrim).c_str());
}
unsigned int Extension::AllClientCount()
{
	return Srv.clientcount();
}
const TCHAR * Extension::GetDenyReason()
{
	return Runtime.CopyString(UTF8ToTString(DenyReason).c_str());
}

/// <summary> Number of UTF-8 code points (including things like combining accents) </summary>
int Extension::ConvToUTF8_GetCompleteCodePointCount(const TCHAR * tStr)
{
	// Empty string
	if (tStr[0] == _T('\0'))
		return 0;

	const std::string u8str = TStringToUTF8(tStr);
	if (u8str.empty())
		return -1;

	size_t numCodePoints = 0;

	const utf8proc_uint8_t * str = (const utf8proc_uint8_t *)u8str.data();
	utf8proc_int32_t thisChar;
	utf8proc_ssize_t numBytesInCodePoint, remainder = u8str.size();
	while (remainder > 0)
	{
		numBytesInCodePoint = utf8proc_iterate(str, remainder, &thisChar);
		if (numBytesInCodePoint <= 0 || !utf8proc_codepoint_valid(thisChar))
			return -1;

		// str[0] - str[numBytesInCodePoint - 1] is one code point
		++numCodePoints;

		str += numBytesInCodePoint;
		remainder -= numBytesInCodePoint;
	}

	return numCodePoints;
}
/// <summary> Get number of UTF-8 graphemes (distinct graphical characters) </summary>
int Extension::ConvToUTF8_GetVisibleCharCount(const TCHAR * tStr)
{
	// Empty string
	if (tStr[0] == _T('\0'))
		return 0;

	const std::string u8str = TStringToUTF8(tStr);
	if (u8str.empty())
		return -1;

	size_t numGraphemes = 0;

	utf8proc_uint8_t * str = (utf8proc_uint8_t *)u8str.data();
	utf8proc_int32_t lastCodePoint = 0, thisCodePoint, state = 0;
	utf8proc_ssize_t numBytesInCodePoint, remainder = u8str.size();
	while (remainder > 0)
	{
		numBytesInCodePoint = utf8proc_iterate(str, remainder, &thisCodePoint);
		if (numBytesInCodePoint <= 0 || !utf8proc_codepoint_valid(thisCodePoint))
			return -1;

		// if utf8proc_grapheme returns true, there is a new grapheme starting between lastCodePoint and thisCodePoint
		if (lastCodePoint != 0 && utf8proc_grapheme_break_stateful(lastCodePoint, thisCodePoint, &state))
			++numGraphemes;
		lastCodePoint = thisCodePoint;

		str += numBytesInCodePoint;
		remainder -= numBytesInCodePoint;
	}

	// Add 1 codePoint for the thisCodePoint at end
	return ++numGraphemes;
}
/// <summary> Get number of bytes in a UTF-8 string </summary>
int Extension::ConvToUTF8_GetByteCount(const TCHAR * tStr)
{
	// Empty string
	if (tStr[0] == _T('\0'))
		return 0;

	// TODO: can make this a little faster by only measuring length of output in WideToMultiByte(),
	// instead of doing that + the conversion itself as TStringToUTF8() does internally.
	const size_t u8size = TStringToUTF8(tStr).size();
	return u8size <= 0 ? -1 : (int)u8size;
}

extern int FindAllowListFromName(const TCHAR * listToSet);
/// <summary> Tests if the UTF-8 equivalent matches the passed allow list, and if allow list is valid.
///			  If so, blank is returned, otherwise the error or faulty character. </summary>
const TCHAR * Extension::ConvToUTF8_TestAllowList(const TCHAR * toTest, const TCHAR * allowList)
{
	lacewing::codepointsallowlist list;
	utf8proc_int32_t rejectedChar = -1;

	// First, allow an already-set allow list to be used by name instead
	int allowListSrvIndex = FindAllowListFromName(allowList);
	if (allowListSrvIndex == -1)
	{
		// Not found by name; try to parse it as a list
		const std::string err = list.setcodepointsallowedlist(TStringToUTF8(allowList));
		if (!err.empty())
			return Runtime.CopyString(UTF8ToTString(err).c_str());

		// fall through
	}

	const int rejectedCodePointIndex = allowListSrvIndex == -1 ?
		list.checkcodepointsallowed(TStringToUTF8(toTest), &rejectedChar) :
		Srv.checkcodepointsallowed((lacewing::relayserver::codepointsallowlistindex)allowListSrvIndex, TStringToUTF8(toTest), &rejectedChar);
	if (rejectedCodePointIndex == -1)
		return Runtime.CopyString(_T(""));

	TCHAR output[256];
	_stprintf_s(output, _T("Code point at index %d does not match allowed list. Code point U+%0.4X, decimal %u; valid = %s, Unicode category = %hs."),
		rejectedCodePointIndex, rejectedChar, rejectedChar, utf8proc_codepoint_valid(rejectedChar) ? _T("yes") : _T("no"), utf8proc_category_string(rejectedChar));
	return Runtime.CopyString(output);
}
