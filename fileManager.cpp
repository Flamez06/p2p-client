#include "fileManager.h"
using namespace std;

bool calculateSHA256(const vector<char>& data, string& hash) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(
        reinterpret_cast<const unsigned char*>(data.data()),
        data.size(),
        digest
    );
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0')
           << static_cast<int>(digest[i]);
    }
    hash = ss.str();
    return true;
}


bool createFileMetadata(const string &filePath, fileMetadata &metadata) {
    ifstream file(filePath, ios::binary | ios::ate);
    if (!file) {
        cout << "Failed to open file: " << filePath << endl;
        return false;
    }
    metadata.fileSize = file.tellg();
    metadata.fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
    metadata.pieceSize = 1024*1024; // 1 MB
    metadata.pieceCount = static_cast<uint32_t>((metadata.fileSize + metadata.pieceSize - 1) / metadata.pieceSize);
    metadata.pieceHashes.clear();

    //Calculate hashes and populate vector
    for(uint32_t i=0;i<metadata.pieceCount;i++){
        vector<char> piece;
        string hash;
        if(!readPiece(filePath,i,metadata.pieceSize,piece)){
            cout << "Failed to read piece " << i << endl;
            return false;
        }
        if(!calculateSHA256(piece,hash)){
            cout << "Failed to calc hash of piece " << i << endl;
            return false;
        }
        metadata.pieceHashes.push_back(hash);
    }
    return true;
}


bool readPiece(const string &filePath, uint32_t pieceIndex, uint32_t pieceSize, vector<char> &piece) {
    ifstream file(filePath, ios::binary);
    if (!file) {
        cout << "Failed to open file: " << filePath << endl;
        return false;
    }
    uint64_t offset = static_cast<uint64_t>(pieceIndex) * pieceSize;
    file.seekg(offset);
    piece.resize(pieceSize);
    file.read(&piece[0], pieceSize);
    streamsize bytesRead = file.gcount();
    piece.resize(bytesRead);
    return bytesRead > 0;
}