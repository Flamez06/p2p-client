#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <vector>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <fstream>

struct fileMetadata {
    std::string fileName;
    uint64_t fileSize;
    uint32_t pieceSize;
    uint32_t pieceCount;
    std::vector<std::string> pieceHashes;
};

bool calculateSHA256(
    const std::vector<char>& data,
    std::string& hash
);

bool createFileMetadata(
    const std::string& filePath,
    fileMetadata& metadata
);

bool readPiece(
    const std::string& filePath,
    uint32_t pieceIndex,
    uint32_t pieceSize,
    std::vector<char>& piece
);

