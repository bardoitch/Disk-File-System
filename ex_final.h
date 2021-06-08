/*--------------------------------------ID: 208241539--------------------------------------------------------------------------------
 ---------------------------------------Name: Bar Doitch------------------------------------------------------------------------------*/

#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
#define DISK_SIZE 256

// ============================================================================
void decToBinary(int n, char &c)
{
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}

// #define SYS_CALL
// ============================================================================
class fsInode
{
    int fileSize;
    int block_in_use;
    int *directBlocks;
    int singleInDirect;
    int num_of_direct_blocks;
    int block_size;
    int single_in_use;

public:
    // ------------------------------------------------------------------------
    fsInode(int _block_size, int _num_of_direct_blocks)
    {
        fileSize = 0;
        block_in_use = 0;
        block_size = _block_size;
        num_of_direct_blocks = _num_of_direct_blocks;
        directBlocks = new int[num_of_direct_blocks];
        assert(directBlocks);
        for (int i = 0; i < num_of_direct_blocks; i++)
        {
            directBlocks[i] = -1;
        }
        singleInDirect = -1;
        single_in_use = 0;
    }
    // ------------------------------------------------------------------------
    int getBlocksInUse()
    {
        return block_in_use;
    }

    // ------------------------------------------------------------------------
    void addBlocksInUse(int size)
    {
        block_in_use += size;
    }

    // ------------------------------------------------------------------------
    int getFileSize()
    {
        return fileSize;
    }

    // ------------------------------------------------------------------------
    void addFileSize(int size)
    {
        fileSize += size;
    }

    // ------------------------------------------------------------------------
    void setDirectBlock(int indexBlock)
    {
        int i = 0;
        while (directBlocks[i] != -1)
        {
            i++;
        }

        directBlocks[i] = indexBlock;
    }

    // ------------------------------------------------------------------------
    int getlastDirect()
    {
        return directBlocks[block_in_use - 1];
    }

    // ------------------------------------------------------------------------
    int getSingleInDirect()
    {
        return singleInDirect;
    }

    // ------------------------------------------------------------------------
    int getDirectByIndex(int index)
    {
        return directBlocks[index];
    }

    // ------------------------------------------------------------------------
    void setSingleInDirect(int index)
    {
        singleInDirect = index;
    }

    // ------------------------------------------------------------------------
    void addSingleInUse(int size)
    {
        single_in_use += size;
    }

    // ------------------------------------------------------------------------
    int getSingleInUse()
    {
        return single_in_use;
    }

    // ------------------------------------------------------------------------
    ~fsInode()
    {
        delete[] directBlocks;
    }
};

// ============================================================================
class FileDescriptor
{
    pair<string, fsInode *> file;
    bool inUse;

public:
    // ------------------------------------------------------------------------
    FileDescriptor(string FileName, fsInode *fsi)
    {
        file.first = FileName;
        file.second = fsi;
        inUse = true;
    }

    // ------------------------------------------------------------------------
    string getFileName()
    {
        return file.first;
    }

    // ------------------------------------------------------------------------
    fsInode *getInode()
    {
        return file.second;
    }

    // ------------------------------------------------------------------------
    bool isInUse()
    {
        return (inUse);
    }

    // ------------------------------------------------------------------------
    void setInUse(bool _inUse)
    {
        inUse = _inUse;
    }

    // ------------------------------------------------------------------------
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk
{
    FILE *sim_disk_fd;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;
    int *BitVector;

    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode *> MainDir;

    // OpenFileDescriptors --  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    vector<FileDescriptor> OpenFileDescriptors;

    int direct_enteris;
    int block_size;
    int disk_blocks_in_use;
    bool is_formated = false; //flag that Checks if this disk is already formatted
    int maxFileSize;          //the max bits in this disk

public:
    // ------------------------------------------------------------------------
    fsDisk()
    {
        sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
        assert(sim_disk_fd);
        for (int i = 0; i < DISK_SIZE; i++)
        {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fwrite("\0", 1, 1, sim_disk_fd);
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
    }

    // ------------------------------------------------------------------------
    void listAll()
    {
        int i = 0;
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it)
        {
            cout << "index: " << i << ": FileName: " << it->getFileName() << " , isInUse: " << it->isInUse() << endl;
            i++;
        }
        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++)
        {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);
            cout << bufy;
        }
        cout << "'" << endl;
    }

    // ------------------------------------------------------------------------
    void fsFormat(int blockSize = 4, int direct_Enteris_ = 3)
    {
        // Check if this file format already
        if (is_formated)
        {
            cout << "ALREADY FORMATTED. " << endl;
            return;
        }

        // Initializing
        block_size = blockSize;
        direct_enteris = direct_Enteris_;
        BitVectorSize = DISK_SIZE / block_size;
        direct_enteris = direct_Enteris_;
        disk_blocks_in_use = 0;
        maxFileSize = (direct_enteris + block_size) * block_size;

        BitVector = new int[BitVectorSize];
        for (int i = 0; i < BitVectorSize; i++)
        {
            BitVector[i] = 0;
        }

        is_formated = true;

        // Output
        cout << "FORMAT DISK: number of blocks: ";
        cout << BitVectorSize << endl;
    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName)
    {
        // Check if this file format already
        if (!is_formated)
        {
            cout << "NEED TO FORMAT FIRST" << endl;
            return -1;
        }

        // Check if file by this name exist already. (file key = file name)
        for (auto it = begin(MainDir); it != end(MainDir); ++it)
        {
            if (fileName.compare(it->first) == 0)
            {
                cout << "FILE ALREADY EXIST. " << endl;
                return -1;
            }
        }

        // Initializing
        fsInode *inode = new fsInode(block_size, direct_enteris);
        FileDescriptor fd(fileName, inode);
        MainDir.insert(pair<string, fsInode *>(fileName, inode));
        int index = MainDir.size() - 1;
        OpenFileDescriptors.push_back(fd);

        return index;
    }

    // ------------------------------------------------------------------------
    int OpenFile(string fileName)
    {
        // Check if this file format already
        if (!is_formated)
        {
            cout << "NEED TO FORMAT FIRST" << endl;
            return -1;
        }

        // Check if this file exist and then open it
        bool exist = false;
        int index = 0;
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it)
        {

            if (fileName.compare(it->getFileName()) == 0)
            {
                exist = true;
                if (it->isInUse())
                {
                    cout << "THIS FILE OPENED ALREADY." << endl;
                    return -1;
                }

                it->setInUse(true);
                return index;
            }

            index++;
        }

        // File name not found
        if (!exist)
        {
            cout << "THIS FILE NOT EXIST." << endl;
            return -1;
        }

        return -1;
    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd)
    {
        // Check if this file format already
        if (!is_formated)
        {
            cout << "NEED TO FORMAT FIRST." << endl;
            return "-1";
        }

        // Check if this file exist and then close it
        bool exist = false;
        int index = 0;
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it)
        {

            if (index == fd)
            {
                exist = true;
                if (!it->isInUse())
                {
                    cout << "THIS FILE CLOSED ALREADY." << endl;
                    return "-1";
                }

                it->setInUse(false);
                return it->getFileName();
            }
            index++;
        }

        //File name not found
        if (!exist)
        {
            cout << "THIS FILE NOT EXIST." << endl;
            return "-1";
        }

        return "-1";
    }
    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char *buf, int len)
    {
        // Check if this file format already
        if (!is_formated)
        {
            cout << "NEED TO FORMAT FIRST." << endl;
            return -1;
        }

        if (len == 0)
            return -1;

        // Check if there is enough empty blocks
        double needed_blocks = (double)len / (double)block_size;

        if ((disk_blocks_in_use + needed_blocks) > BitVectorSize)
        {
            cout << "THERE IS NO ENOUGH EMPTY BLOCKS." << endl;
            return -1;
        }

        // Check if this file exist
        bool exist = false;
        int index = 0;
        vector<FileDescriptor>::iterator Vit;

        for (Vit = begin(OpenFileDescriptors); Vit != end(OpenFileDescriptors); ++Vit)
        {
            if (index == fd)
            {
                exist = true;
                if (!Vit->isInUse())
                {
                    cout << "THIS FILE CLOSED ALREADY." << endl;
                    return -1;
                }
                break;
            }
            index++;
        }

        //File name not found
        if (!exist)
        {
            cout << "THIS FILE NOT EXIST." << endl;
            return -1;
        }

        // Find this file in the MainDir
        map<string, fsInode *>::iterator Mit;
        for (Mit = begin(MainDir); Mit != end(MainDir); ++Mit)
        {
            if (Vit->getFileName().compare(Mit->first) == 0)
            {
                break;
            }
        }

        // Check if this file has enough empty blocks
        int fileSize = Mit->second->getFileSize();
        if ((fileSize + len) > maxFileSize)
        {
            cout << "THERE IS NO ENOUGH EMPTY BLOCKS IN THIS FILE." << endl;
            return -1;
        }

        //  After all the tests(if this file pass this), it is now possible to write to this file

        int blocksInUse = Vit->getInode()->getBlocksInUse();
        fsInode *inode = Mit->second;

        // This is the first writing of this file
        if (blocksInUse == 0)
        {
            writeToNewBlock(buf, len, inode);
            return fd;
        }

        // This is not the first writing of this file
        else if (blocksInUse <= direct_enteris)
        {
            writeToExistBlock(buf, len, inode);
            return fd;
        }

        // The direct block of this file is full
        else if (blocksInUse > direct_enteris)
        {
            // This is the first writing to the single in direct blocks of this file
            if (inode->getSingleInDirect() == -1)
            {
                writeToNewSingleBlocks(buf, len, inode);
                return fd;
            }

            // This is not the first writing to the single in direct blocks of this file
            else
                writeToSingleBlocks(buf, len, inode);
            return fd;
        }

        return fd;
    }
    // -----------------------------------------------------------------------
    void writeToExistBlock(char *buf, int len, fsInode *inode)
    {

        int fileSize = inode->getFileSize();
        int blocksInUse = inode->getBlocksInUse();

        //  The direct blocks of this file is full
        if (fileSize >= direct_enteris * block_size)
        {
            writeToSingleBlocks(buf, len, inode);
            return;
        }

        // There is empty bits in the last assigned direct block
        else if (fileSize < direct_enteris * block_size)
        {

            int currBlock = inode->getlastDirect();                //the last direct block that allocated
            int emptyBits = (blocksInUse * block_size) - fileSize; //some empty bits in the last direct

            int lenOfTemp = len;
            if (len > emptyBits)
            {
                lenOfTemp = emptyBits;
            }

            // This temp string will added to this direct block
            char temp[lenOfTemp];

            int bufIndex = 0; //index of buffer
            for (int j = 0; j < lenOfTemp; j++)
            {
                temp[j] = buf[bufIndex];
                bufIndex++;
            }

            int index = (currBlock * block_size) + (block_size - emptyBits); //current index in this file

            fseek(sim_disk_fd, index, SEEK_SET);
            fwrite(temp, sizeof(char), lenOfTemp, sim_disk_fd);

            // If write is done.
            if (len <= emptyBits)
            {
                inode->addFileSize(len);
                return;
            }

            // Updets
            inode->addFileSize(lenOfTemp);

            // If this string(the buffer) is longer and need more blocks
            int new_len = len - lenOfTemp;
            char new_buf[new_len];
            for (int i = 0; i < new_len; i++)
            {
                new_buf[i] = buf[emptyBits];
                emptyBits++;
            }

            writeToNewBlock(new_buf, new_len, inode);
            return;
        }

        writeToNewBlock(buf, len, inode);
    }
    // ------------------------------------------------------------------------
    void writeToNewBlock(char *buf, int len, fsInode *inode)
    {
        if (len <= 0)
            return;

        int indOfBlock;      // index of block
        int inFileIndex = 0; // offset in file
        int offset;          // length of the temp buffer
        int bufIndex = 0;    // index of the original buffer

        for (indOfBlock = 0; indOfBlock < BitVectorSize; indOfBlock++)
        {
            int blockInUse = inode->getBlocksInUse();

            // Direct blocks of this file is full
            if (blockInUse >= direct_enteris)
            {
                writeToNewSingleBlocks(buf + bufIndex, len - bufIndex, inode);
                return;
            }

            // Empty block is found
            if (BitVector[indOfBlock] != 1)
            {
                // This string(the buffer) can be longer than block size, or shorter
                offset = block_size;
                if (len - bufIndex < block_size)
                {
                    offset = len - bufIndex;
                }

                // This temp string will added to new block
                char temp[offset];

                int i = bufIndex;
                for (int j = 0; j < offset; j++)
                {
                    temp[j] = buf[i];
                    i++;
                }

                inFileIndex = indOfBlock * block_size;
                fseek(sim_disk_fd, inFileIndex, SEEK_SET);
                fwrite(temp, 1, offset, sim_disk_fd);

                //Updets
                bufIndex += offset;
                inode->addFileSize(offset);
                inode->addBlocksInUse(1);
                inode->setDirectBlock(indOfBlock);
                BitVector[indOfBlock] = 1;
                disk_blocks_in_use += 1;

                if (bufIndex >= len)
                {
                    break;
                }
            }
        }
    }

    // ------------------------------------------------------------------------
    void writeToSingleBlocks(char *buf, int len, fsInode *inode)
    {
        int fileSize = inode->getFileSize();
        int blocksInUse = inode->getBlocksInUse();

        // The single blocks that already been allocated is not full
        if (blocksInUse * block_size > fileSize)
        {
            int indexPointer = 0;
            char t = '0';
            int size = (blocksInUse * block_size) - fileSize;
            int indexInFile = (inode->getSingleInDirect() * block_size) + inode->getSingleInUse() - 1;
            int offset = block_size - size;

            // Find the block index of this allocted single direct block
            fseek(sim_disk_fd, indexInFile, SEEK_SET);
            fread(&t, sizeof(t), 1, sim_disk_fd);
            indexPointer = (int)(t)-48; //assci value

            // This string(the buffer) can be longer than block size, or shorter
            int new_len = size;
            if (len < size)
                new_len = len;

            // This temp string will added to this block
            char temp[new_len];
            for (int i = 0; i < new_len; i++)
            {
                temp[i] = buf[i];
            }

            fseek(sim_disk_fd, (indexPointer * block_size) + offset, SEEK_SET);
            fwrite(temp, 1, new_len, sim_disk_fd);

            //Updets
            inode->addFileSize(new_len);

            buf = buf + new_len;
            len = len - new_len;
        }

        writeToNewSingleBlocks(buf, len, inode);
    }

    // ------------------------------------------------------------------------
    void writeToNewSingleBlocks(char *buf, int len, fsInode *inode)
    {
        if (len <= 0)
        {
            return;
        }

        // Need to allocate single blocks to this file
        if (inode->getSingleInDirect() == -1)
        {
            int index_of_single = -1;
            for (index_of_single = 0; index_of_single < BitVectorSize; index_of_single++)
            {
                // Empty block is found
                if (BitVector[index_of_single] == 0)
                {
                    inode->setSingleInDirect(index_of_single);
                    BitVector[index_of_single] = 1;
                    inode->addBlocksInUse(1);
                    disk_blocks_in_use += 1;
                    break;
                }
            }
        }

        int bufIndex = 0; //index of buffer
        int numOfBlock;   //index of block
        int inFileIndex = 0;
        int toFileIndex = block_size;
        int offset;
        for (numOfBlock = 0; numOfBlock < BitVectorSize; numOfBlock++)
        {
            // Empty block is found
            if (BitVector[numOfBlock] != 1)
            {

                // This string (the buffer) can be longer than block size, or shorter
                offset = block_size;
                if (len - bufIndex < block_size)
                {
                    offset = len - bufIndex;
                }

                // This temp string will added to new block
                char temp[offset];

                int i = bufIndex;
                for (int j = 0; j < offset; j++)
                {
                    temp[j] = buf[i];
                    i++;
                }

                inFileIndex = numOfBlock * block_size;
                toFileIndex = (numOfBlock * block_size) + offset;
                fseek(sim_disk_fd, inFileIndex, SEEK_SET);
                fwrite(temp, 1, offset, sim_disk_fd);

                // Updets
                bufIndex += offset;

                inode->addFileSize(offset);
                inode->addBlocksInUse(1);

                int singleIndex = inode->getSingleInDirect();
                int numOfSingle = inode->getSingleInUse();

                // Add pointer to this new block in the single in direct block
                char c = '0';
                int f_index = (singleIndex * block_size) + numOfSingle;
                decToBinary(numOfBlock, c);
                fseek(sim_disk_fd, f_index, SEEK_SET);

                char s[1];
                s[0] = c;
                fwrite(s, 1, 1, sim_disk_fd);

                BitVector[numOfBlock] = 1;
                disk_blocks_in_use += 1;
                inode->addSingleInUse(1);

                if (bufIndex >= len)
                {
                    break;
                }
            }
        }
    }

    // ------------------------------------------------------------------------
    int DelFile(string FileName)
    {
        // Check if this file format already
        if (!is_formated)
        {
            cout << "NEED TO FORMAT FIRST." << endl;
            return -1;
        }

        // Find this file in the MainDir
        map<string, fsInode *>::iterator Mit;
        for (Mit = begin(MainDir); Mit != end(MainDir); ++Mit)
        {
            if (FileName.compare(Mit->first) == 0)
            {
                break;
            }
        }

        // Check if this file exist
        bool exist = false;
        int Vindex = 0; //index of this file in OpenFileDescriptors vector
        vector<FileDescriptor>::iterator Vit;

        for (Vit = begin(OpenFileDescriptors); Vit != end(OpenFileDescriptors); ++Vit)
        {
            if (Vit->getFileName().compare(FileName) == 0)
            {
                exist = true;
                if (!Vit->isInUse())
                {
                    cout << "THIS FILE CLOSED ALREADY." << endl;
                    return -1;
                }

                break;
            }

            Vindex++;
        }

        // File name not found
        if (!exist)
        {
            cout << "THIS FILE NOT EXIST." << endl;
            return -1;
        }

        // In this loop find this file's blocks and delete them
        fsInode *inode = Mit->second;
        int len = inode->getFileSize();

        for (int index = 0; index < len; index++)
        {
            // Delete from direct blocks
            if (index < (direct_enteris * block_size))
            {
                int block_index = inode->getDirectByIndex((index / block_size));
                int Bit_index = block_index * block_size + (index % block_size);

                fseek(sim_disk_fd, Bit_index, SEEK_SET);
                fwrite("\0", 1, 1, sim_disk_fd);
            }

            // Delete from single in direct blocks
            else
            {
                int single_block_index = inode->getSingleInDirect();
                int p_index = (index / block_size) - direct_enteris; // the pointer index in the single block

                fseek(sim_disk_fd, (single_block_index * block_size) + p_index, SEEK_SET);
                char c = '0';
                fread(&c, sizeof(c), 1, sim_disk_fd);

                int index_Pointer = (int)(c)-48; //assci value

                int ind = index_Pointer * block_size + (index % block_size);

                fseek(sim_disk_fd, ind, SEEK_SET);
                fwrite("\0", 1, 1, sim_disk_fd);
            }
        }

        // Delete the single block
        int single_block_index = inode->getSingleInDirect();
        fseek(sim_disk_fd, single_block_index * block_size, SEEK_SET);

        int singleInUse = inode->getSingleInUse();
        int i = inode->getSingleInDirect();
        for (int index = 0; index < singleInUse; index++)
        {
            fseek(sim_disk_fd, i * block_size + index, SEEK_SET);
            fwrite("\0", 1, 1, sim_disk_fd);
        }

        //Updates
        disk_blocks_in_use -= Mit->second->getBlocksInUse() ;
        delete Mit->second;
        MainDir.erase(Mit);
        OpenFileDescriptors.erase(Vit);

        return Vindex;
    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len)
    {
        // Check if this file format already
        if (!is_formated)
        {
            cout << "NEED TO FORMAT FIRST." << endl;
            return -1;
        }

        if (len <= 0)
            return -1;

        // Check if this file exist and then read from it
        bool exist = false;
        int index = 0;
        vector<FileDescriptor>::iterator Vit;

        for (Vit = begin(OpenFileDescriptors); Vit != end(OpenFileDescriptors); ++Vit)
        {
            if (index == fd)
            {
                exist = true;
                if (!Vit->isInUse())
                {
                    cout << "THIS FILE CLOSED ALREADY." << endl;
                    return -1;
                }

                break;
            }
            index++;
        }

        // File name not found
        if (!exist)
        {
            cout << "THIS FILE NOT EXIST." << endl;
            return -1;
        }

        // Find this file in the MainDir
        map<string, fsInode *>::iterator Mit;
        for (Mit = begin(MainDir); Mit != end(MainDir); ++Mit)
        {
            if (Vit->getFileName().compare(Mit->first) == 0)
                break;
        }

        fsInode *inode = Mit->second;
        int file_size = inode->getFileSize();
        int len_to_read = len;

        if (len_to_read > file_size)
        {
            len_to_read = file_size;
        }

        for (index = 0; index < len_to_read; index++)
        {
            char temp[1];
            // Read from direct blocks
            if (index < (direct_enteris * block_size))
            {
                int block_index = inode->getDirectByIndex((index / block_size));
                int Bit_index = block_index * block_size + (index % block_size);

                fseek(sim_disk_fd, Bit_index, SEEK_SET);
                fread(temp, sizeof(temp), 1, sim_disk_fd);
                buf[index] = temp[0];
                buf[index + 1] = '\0';
            }

            // Read from single in direct blocks
            else
            {
                char temp[1];

                int single_block_index = inode->getSingleInDirect();

                int p_index = (index - block_size * direct_enteris) / block_size;
                fseek(sim_disk_fd, (single_block_index * block_size) + p_index, SEEK_SET);
                char c = '0';
                fread(&c, sizeof(c), 1, sim_disk_fd);

                int index_Pointer = (int)(c)-48; //assci value

                int ind = index_Pointer * block_size + (index % block_size);

                fseek(sim_disk_fd, ind, SEEK_SET);
                fread(temp, sizeof(temp), 1, sim_disk_fd);
                buf[index] = temp[0];
                buf[index + 1] = '\0';
            }
        }
    }
    // ------------------------------------------------------------------------

    ~fsDisk()
    {
        // Delete all the files in the MainDir
        map<string, fsInode *>::iterator Mit;
        for (Mit = begin(MainDir); Mit != end(MainDir); ++Mit)
        {
            delete Mit->second;
        }

        // Close this disk
        fclose(sim_disk_fd);

        // Delete this pointer
        if (is_formated)
            delete[] BitVector;
    }

    // ------------------------------------------------------------------------
};
