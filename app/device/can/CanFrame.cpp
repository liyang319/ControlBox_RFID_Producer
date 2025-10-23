#include "CanFrame.h"
#include "Base.h"

using namespace std;

CanFrame::CanFrame()
{
    CanFrame(CAN_TYPE_CAN_20);
}

CanFrame::CanFrame(int canType)
{
    this->canType = canType;
    if (canType == CAN_TYPE_CAN_20)
        this->length = 8;
    else
        this->length = 64;

    this->id = 0;
    this->isRemote = false;
    this->isExtend = false;

    if (canType == CAN_TYPE_CAN_20)
    {
        for (int i = 0; i < this->length; ++i) 
        {
            this->data[i] = 0;
        }
    }
    else
    {
        for (int i = 0; i < this->length; ++i) 
        {
            this->datafd[i] = 0;
        }
    }
}

CanFrame::~CanFrame()
{
}

/**
 * insert a signal into the CAN frame
 */
bool CanFrame::insertSignal(const CanSignal &signal)
{
    uint8_t i = 0;
    uint8_t bytenum = 0;
    uint8_t lowbyte = 0;
    uint8_t highbyte = 0;
    uint8_t lowbytepos = 0;
    uint8_t highbytepos = 0;
    uint8_t remainbitnum = 0;
    uint8_t remainbytenum = 0;	
    uint64_t signalmask = SIGNAL_INIT_MASK;

    if (signal.bitSize > length * 8 || signal.bitSize == SIGNAL_ZERO_LENGTH || ((canType == CAN_TYPE_CAN_20 && signal.bitSize > SIGNAL_MAX_LENGTH) || ((canType == CAN_TYPE_CAN_FD && signal.bitSize > SIGNAL_MAX_FD_LENGTH))))
    {
        // signal valid?
        COUT << "signal bitSize invalid: " << to_string(signal.bitSize) << endl;
        COUT << "length: " << to_string(length) << endl;
        return false;
    }
    else if ((canType == CAN_TYPE_CAN_20 && signal.startBit >= SIGNAL_MAX_LENGTH) || (canType == CAN_TYPE_CAN_FD && signal.startBit >= SIGNAL_MAX_FD_LENGTH))
    {
        // signal valid?
        COUT << "signal startbit invalid: " << to_string(signal.startBit) << endl;
        return false;
    }

    uint8_t* pdata;

    if (canType == CAN_TYPE_CAN_20)
        pdata = this->data;
    else
        pdata = this->datafd;

    //
    if (signal.byteOrder == SIGNAL_BYTE_ORDER_INTEL)
    {
        //COUT << "小端字节序" << endl;

        // Byte Order: Intel
        lowbyte = signal.startBit / 8;
        highbyte = (signal.startBit + signal.bitSize - 1) / 8;
        
        if ((highbyte + 1) > length)
        {
            // CAN signal is not in the range of DLC
            COUT << "signal startbit + bitSize is not in the range of DLC!" << endl;
            return false;
        }
        else
        {
            for (i = 0;i < signal.bitSize; i++)
            {
                // Generate signal mask according to signal bitSize
                signalmask |= 0x0000000000000001 << i;
            }

            // overflow
            uint64_t value = signal.value;
            if (value > signalmask) 
            {
                value = value & signalmask;
            }

            // Distance from the lowest bit of each byte
            lowbytepos = signal.startBit % 8;
            
            if (lowbyte == highbyte)
            {
                // CAN Signal is in a byte
                uint8_t mask = ~(signalmask << lowbytepos);
                pdata[lowbyte] = (pdata[lowbyte] & mask) + (value << lowbytepos);
            }
            else
            {
                // CAN Signal in multiple bytes
                for (bytenum=lowbyte; bytenum<=highbyte; bytenum++)
                {
                    if (bytenum == lowbyte)
                    {
                        // low byte
                        uint8_t mask = ~((signalmask << lowbytepos) & 0xff);
                        pdata[bytenum] = (pdata[bytenum] & mask) + ((value << lowbytepos) & 0xff);
                    }
                    else if (bytenum < highbyte)
                    {
                        // middle byte
                        pdata[bytenum] = (value >> ((bytenum - lowbyte - 1) * 8 + (8 - lowbytepos))) & 0xff;
                    }
                    else 
                    {
                        // high byte
                        uint8_t highbytevalue = (value >> ((highbyte - lowbyte - 1) * 8 + (8 - lowbytepos))) & 0xff;
                        uint8_t mask = ~((signalmask >> ((bytenum - lowbyte - 1) * 8 + (8 - lowbytepos))) & 0xff);
                        pdata[bytenum] = (pdata[bytenum] & mask) + highbytevalue;
                    }
                }
            }
        }
    }
    else
    {
        //COUT << "大端字节序" << endl;

        // Byte Order: Motorola
        highbyte = signal.startBit / 8;
        
        if ((highbyte + 1) > length)
        {
            // CAN signal is not in the range of DLC
            COUT << "signal startbit + bitSize is not in the range of DLC!" << endl;
            return false;
        }
        else
        {
            for (i = 0; i < signal.bitSize; i++)
            {
                // Generate signal mask according to signal bitSize
                signalmask |= 0x0000000000000001 << i;
            }

            // overflow
            uint64_t value = signal.value;
            if (value > signalmask) 
            {
                value = value & signalmask;
            }
            
            // Distance from the lowest bit of each byte
            highbytepos = signal.startBit % 8;
            
            if (signal.bitSize <= (8 - highbytepos))
            {
                // CAN Signal is in a bytes	
                uint8_t mask = ~(signalmask << highbytepos);
                pdata[highbyte] = (pdata[highbyte] & mask) + (value << highbytepos);
            }
            else
            {
                // CAN Signal in multiple bytes : example signal startbit=19, bitSize=16
                remainbitnum = signal.bitSize - (8 - highbytepos);       // 剩余bit : 16 - (8 - 3) = 11
                
                if ((remainbitnum % 8) != 0)
                {
                    remainbytenum = remainbitnum / 8 + 1;               // remainbytenum = 2
                }
                else
                {
                    remainbytenum = remainbitnum / 8;                   
                }

                lowbyte = highbyte - remainbytenum;

                for (bytenum = highbyte; bytenum >= lowbyte; bytenum--)  // highbyte = 2  remainbytenum = 2
                {
                    if (bytenum == highbyte)
                    {
                        // high byte
                        uint8_t mask = ~((signalmask << highbytepos) & 0xff);
                        pdata[bytenum] = (pdata[bytenum] & mask) + ((value << highbytepos) & 0xff);
                    }
                    else if (bytenum > lowbyte)
                    {
                        // middle byte
                        pdata[bytenum] = (value >> ((bytenum - lowbyte - 1) * 8 + (8 - highbytepos))) & 0xff;
                    }
                    else
                    {
                        // low byte
                        uint8_t lowbytevalue = (value >> ((highbyte - lowbyte - 1) * 8 + (8 - highbytepos))) & 0xff;
                        uint8_t mask = ~((signalmask >> ((bytenum - lowbyte - 1) * 8 + (8 - highbytepos))) & 0xff);
                        pdata[bytenum] = (pdata[bytenum] & mask) + lowbytevalue;
                    }
                }								
            }			
        }	
    }

    // length
    int l = 7;
    if (canType == CAN_TYPE_CAN_FD)
    {
        l = 63;
    }

    for (int i = l; i >= 0; --i)
    {
        if (pdata[i] != 0)
        {
            realLength = i+1;
            break;
        }
    } 

    // COUT << "FRAME: " << endl;
    // for (int i = 0; i < realLength; ++i) 
    // {
    //     cout << hex << setw(2) << setfill('0') << static_cast<int>(pdata[i]) << " ";
    // }
    // cout << endl;

    return true;
}

/**
 * Get the signal value from the CAN frame
 */
CanSignal* CanFrame::getSignal(uint8_t startbit, uint8_t bitsize, uint8_t byteorder)
{
    CanSignal *signal = new CanSignal();
    uint8_t i = 0;
    uint8_t bytenum = 0;
    uint8_t lowbyte = 0;
    uint8_t highbyte = 0;
    uint8_t lowbytepos = 0;
    uint8_t highbytepos = 0;
    uint8_t remainbitnum = 0;
    uint8_t remainbytenum = 0;	
    uint64_t signalmask = SIGNAL_INIT_MASK;
    
    // init
    signal->state = CAN_SIGNAL_NORMAL;
    signal->value = SIGNAL_INIT_VALUE;
    signal->startBit = startbit;
    signal->bitSize = bitsize;
    
    if (this->length == FRAME_ZERO_DLC)
    {
        //DLC is 0
        COUT << "DLC is 0" << endl;
        signal->state = CAN_SIGNAL_NO_VALUE;
        return signal;
    }
    else if ((this->canType == CAN_TYPE_CAN_20 && this->length > FRAME_MAX_DLC) || (this->canType == CAN_TYPE_CAN_FD && this->length > FRAME_MAX_FD_DLC))
    {
        //DLC is out of range
        COUT << "DLC is out of range" << endl;
        signal->state = CAN_FRAME_DLC_OUT_OF_RANGE;
        return signal;
    }
    else if (bitsize > this->length * 8 || bitsize == SIGNAL_ZERO_LENGTH || ((this->canType == CAN_TYPE_CAN_20 && bitsize > SIGNAL_MAX_LENGTH) || (this->canType == CAN_TYPE_CAN_FD && bitsize > SIGNAL_MAX_FD_LENGTH)))
    {
        // signal valid?
        COUT << "signal value invalid!" << endl;
        signal->state = INVALID_INPUT_PARAMETERS;
        return signal;
    }

    uint8_t* pdata;

    if (canType == CAN_TYPE_CAN_20)
        pdata = this->data;
    else
        pdata = this->datafd;
    
    if (byteorder == SIGNAL_BYTE_ORDER_INTEL)
    {
        // Byte Order: Intel
        lowbyte = startbit / 8;
        highbyte = (startbit + bitsize - 1) / 8;  // example: (2+6-1)/8=0; (2+7-1)/8=1; (2+14-1)/8=1; (2+15-1)/8=2 .....
        
        if ((highbyte + 1) > this->length)
        {
            // CAN signal is not in the range of DLC
            signal->state = CAN_FRAME_DLC_OUT_OF_RANGE;
            COUT << "signal value too big!" << endl;
        }
        else
        {
            for (i = 0;i < bitsize; i++)
            {
                // Generate signal mask according to signal bitSize
                signalmask |= 0x0000000000000001 << i;
            }
            
            // Distance from the lowest bit of each byte
            lowbytepos = startbit % 8;
            
            if (lowbyte == highbyte)
            {
                // CAN Signal is in a bytes
                signal->value = (((uint64_t)pdata[lowbyte]) >> lowbytepos) & signalmask;
            }
            else
            {
                // CAN Signal in multiple bytes
                for (bytenum=lowbyte; bytenum<=highbyte; bytenum++)
                {
                    if (bytenum == lowbyte)
                    {
                        signal->value |= ((uint64_t)pdata[lowbyte]) >> lowbytepos;
                    }
                    else
                    {
                        signal->value |= ((uint64_t)pdata[bytenum]) << ((bytenum - lowbyte - 1) * 8 + (8 - lowbytepos));
                    }
                }
                
                signal->value = signal->value & signalmask;
            }
        }
    }
    else
    {
        // Byte Order: Motorola
        highbyte = startbit / 8;
        
        if ((highbyte + 1) > this->length)
        {
            // CAN signal is not in the range of DLC
            signal->state = CAN_FRAME_DLC_OUT_OF_RANGE;
        }
        else
        {
            for (i = 0; i < bitsize; i++)
            {
                // Generate signal mask according to signal bitSize
                signalmask |= 0x0000000000000001 << i;
            }
            
            // Distance from the lowest bit of each byte
            highbytepos = startbit % 8;
            
            if (bitsize <= (8 - highbytepos))
            {
                // CAN Signal is in a bytes	
                signal->value = (((uint64_t)pdata[highbyte]) >> highbytepos) & signalmask;
            }
            else
            {
                // CAN Signal in multiple bytes
                remainbitnum = bitsize - (8 - highbytepos);          // 剩余bit = 12 - (8 - 2) = 6
                
                if((remainbitnum % 8) != 0)
                {
                    remainbytenum = remainbitnum / 8 + 1;           // remainbytenum = 1
                }
                else
                {
                    remainbytenum = remainbitnum / 8;
                }

                for (bytenum = highbyte; bytenum >= (highbyte - remainbytenum); bytenum--)  // highbyte = 2  remainbytenum = 1
                {
                    if(bytenum == highbyte)
                    {
                        signal->value |= ((uint64_t)pdata[highbyte]) >> highbytepos;
                    }
                    else
                    {
                        signal->value |= ((uint64_t)pdata[bytenum]) << ((highbyte - bytenum - 1) * 8 + (8 - highbytepos));
                    }

                    if(bytenum == 0)
                    {
                        break;
                    }
                }

                signal->value = signal->value & signalmask;									
            }			
        }				
    }

    return signal;
}

uint8_t CanFrame::getRealFrameLength()
{
    return this->realLength;
}

uint8_t* CanFrame::getData()
{
    if (canType == CAN_TYPE_CAN_20)
        return this->data;
    else
        return this->datafd;
}

void CanFrame::setData(uint8_t* buffer, size_t size)
{
    if (canType == CAN_TYPE_CAN_20)
        memcpy(this->data, buffer, size);
    else
        memcpy(this->datafd, buffer, size);
}

int CanFrame::getType()
{
    return canType;
}
