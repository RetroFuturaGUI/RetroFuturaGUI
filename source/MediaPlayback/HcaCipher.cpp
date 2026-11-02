#include "HcaCipher.hpp"

bool RetroFuturaGUI::HcaCipher::Init(const u32 cipherType, const HcaKey& key)
{
    _isEncrypted = false;

    switch(cipherType)
    {
        case 0:
            return true;
        case 1:
            initFixedTable();
            _isEncrypted = true;
            return true;
        case 56:
        {
            if(0 == key._KeyCode)
                return false;

            //A file out of an AWB archive mixes the archive's subkey into the keycode
            const u64 keyCode { 0 == key._Subkey ? key._KeyCode : key._KeyCode * ((static_cast<u64>(key._Subkey) << 16) | static_cast<u16>(~key._Subkey + 2u)) };
            initKeyedTable(keyCode);
            _isEncrypted = true;
            return true;
        }
        default:
            return false;
    }
}

bool RetroFuturaGUI::HcaCipher::IsEncrypted() const
{
    return _isEncrypted;
}

void RetroFuturaGUI::HcaCipher::Decrypt(std::span<u8> block) const
{
    if(!_isEncrypted)
        return;

    for(u8& byte : block)
        byte = _table[byte];
}

void RetroFuturaGUI::HcaCipher::createNibbleTable(std::array<u8, 16>* table, u8 key)
{
    if(!table)
        return;

    const u32
        multiplier { static_cast<u32>((key & 1) << 3) | 5 },
        addend { static_cast<u32>(key & 0xE) | 1 };
    key >>= 4;

    for(u8& entry : *table)
    {
        key = static_cast<u8>((key * multiplier + addend) & 0xF);
        entry = key;
    }
}

void RetroFuturaGUI::HcaCipher::initFixedTable()
{
    u32 value { 0 };

    for(uSize entry = 1; entry < 0xFF; ++entry) //keep 0x00 and 0xFF
    {
        value = (value * 13 + 11) & 0xFF;

        if(0 == value || 0xFF == value)
            value = (value * 13 + 11) & 0xFF;

        _table[entry] = static_cast<u8>(value);
    }

    _table[0] = 0;
    _table[0xFF] = 0xFF;
}

void RetroFuturaGUI::HcaCipher::initKeyedTable(u64 keyCode)
{
    std::array<u8, 7> keyBytes {}; //The keycode's lower 56 bits

    if(0 != keyCode)
        --keyCode;

    for(u8& keyByte : keyBytes)
    {
        keyByte = static_cast<u8>(keyCode & 0xFF);
        keyCode >>= 8;
    }

    const std::array<u8, 16> seeds
    {
        keyBytes[1], static_cast<u8>(keyBytes[1] ^ keyBytes[6]), static_cast<u8>(keyBytes[2] ^ keyBytes[3]), keyBytes[2],
        static_cast<u8>(keyBytes[2] ^ keyBytes[1]), static_cast<u8>(keyBytes[3] ^ keyBytes[4]), keyBytes[3], static_cast<u8>(keyBytes[3] ^ keyBytes[2]),
        static_cast<u8>(keyBytes[4] ^ keyBytes[5]), keyBytes[4], static_cast<u8>(keyBytes[4] ^ keyBytes[3]), static_cast<u8>(keyBytes[5] ^ keyBytes[6]),
        keyBytes[5], static_cast<u8>(keyBytes[5] ^ keyBytes[4]), static_cast<u8>(keyBytes[6] ^ keyBytes[1]), keyBytes[6]
    };

    //Each byte of the base table is a row's high nibble and a column's low one
    std::array<u8, 256> base {};
    std::array<u8, 16> rows {}, columns {};
    createNibbleTable(&rows, keyBytes[0]);

    for(uSize row = 0; row < 16; ++row)
    {
        createNibbleTable(&columns, seeds[row]);

        for(uSize column = 0; column < 16; ++column)
            base[row * 16 + column] = static_cast<u8>(rows[row] << 4 | columns[column]);
    }

    //Shuffled by stepping 17 through it, 0x00 and 0xFF left still remain untouched
    uSize step { 0 }, entry { 1 };

    for(uSize count = 0; count < 256 && entry < 0xFF; ++count)
    {
        step = (step + 17) & 0xFF;

        if(0 != base[step] && 0xFF != base[step])
            _table[entry++] = base[step];
    }

    _table[0] = 0;
    _table[0xFF] = 0xFF;
}