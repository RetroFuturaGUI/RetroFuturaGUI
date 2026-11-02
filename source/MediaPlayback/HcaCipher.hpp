#pragma once
#include "config.hpp"
#include <array>
#include <span>

namespace RetroFuturaGUI
{
    /// @brief The key an encrypted HCA file (cipher type 56) needs: the game's 64-bit keycode, and for files out of an AWB
    ///        archive the 16-bit subkey. Keys are unique per game (collections such as vgmstream's list them)
    struct HcaKey
    {
        u64 _KeyCode { 0 };
        u16 _Subkey { 0 };
    };

    /// @brief Decrypts HCA blocks before they reach FFmpeg, whose decoder (7.1.1) misreads the header past the comp chunk and
    ///        therefore never decrypts. Every byte of a block goes through one table (0x00 and 0xFF stay as they are, so a block's  0xFFFF sync stays in-tact)
    class HcaCipher
    {
    public:
        /// @brief Builds the table for a header's cipher type: 0 none, 1 CRI's fixed table, 56 keyed.
        /// @return false for an unknown type or type 56 without a keycode.
        bool Init(const u32 cipherType, const HcaKey& key);

        /// @brief Whether blocks need decrypting at all.
        bool IsEncrypted() const;

        /// @brief Decrypts a block in place.
        void Decrypt(std::span<u8> block) const;

    private:
        /// @brief One of the 16-entry nibble tables type 56 is built from, as FFmpeg's hcadec.c builds them.
        static void createNibbleTable(std::array<u8, 16>* table, u8 key);
        void initFixedTable();
        void initKeyedTable(u64 keyCode);

        std::array<u8, 256> _table {};
        bool _isEncrypted { false };
    };
}