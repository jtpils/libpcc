//
// Created by basti on 25.12.17.
//

#ifndef BITVECTOR_HPP
#define BITVECTOR_HPP

#include <cstdlib>
#include <bitset>

struct AbstractBitVec {
    AbstractBitVec() = default;

    virtual ~AbstractBitVec() = default;

    virtual size_t getSize() = 0;

    virtual size_t getSizeX() = 0;

    virtual size_t getSizeY() = 0;

    virtual size_t getSizeZ() = 0;
};

template <size_t NX, size_t NY, size_t NZ>
struct BitVec : AbstractBitVec {
    explicit BitVec(uint64_t t_x = 0, uint64_t t_y = 0, uint64_t t_z = 0)
        : AbstractBitVec()
        , x(t_x)
        , y(t_y)
        , z(t_z)
    {}

    explicit BitVec(const std::bitset<NX+NY+NZ>& packed)
        : AbstractBitVec()
        , x()
        , y()
        , z()
    {
        setFromPacked(packed);
    }

    ~BitVec() override = default;

    size_t getSize() override {
        return NX + NY + NZ;
    }

    size_t getSizeX() override {
        return NX;
    }

    size_t getSizeY() override {
        return NY;
    }

    size_t getSizeZ() override {
        return NZ;
    }

    const std::bitset<NX+NY+NZ> getPacked() {
        std::bitset<NX+NY+NZ> packed;
        for(size_t i = 0; i < NX; ++i)
            packed[i] = x[i];
        for(size_t i = 0; i < NY; ++i)
            packed[NX+i] = y[i];
        for(size_t i = 0; i < NZ; ++i)
            packed[NX+NY+i] = x[i];
        return packed;
    }

    void setFromPacked(const std::bitset<NX+NY+NZ>& packed) {
        for(size_t i = 0; i < NX; ++i)
            x[i] = packed[i];
        for(size_t i = 0; i < NY; ++i)
            y[i] = packed[NX+i];
        for(size_t i = 0; i < NZ; ++i)
            z[i] = packed[NX+NY+i];
    }

    std::bitset<NX> x;
    std::bitset<NY> y;
    std::bitset<NZ> z;
};

template <size_t N>
struct UniformBitVec : BitVec<N,N,N> {};

struct AbstractBitVecArray {
    AbstractBitVecArray() = default;

    virtual ~AbstractBitVecArray() = default;

    virtual size_t getByteSize() = 0;

    virtual size_t getNX() = 0;

    virtual size_t getNY() = 0;

    virtual size_t getNZ() = 0;
};

template <size_t NX, size_t NY, size_t NZ>
struct BitVecArray : AbstractBitVecArray {
    BitVecArray()
        : AbstractBitVecArray()
        , data()
        , packed_data(nullptr)
    {}

    ~BitVecArray() override
    {
        if(packed_data != nullptr)
            delete [] packed_data;
    }

    size_t getByteSize() override
    {
        size_t bit_size = data.size() * (NX+NY+NZ);
        return static_cast<size_t>(ceil(bit_size/8.0f));
    }

    size_t getNX() override {
        return NX;
    }

    size_t getNY() override {
        return NY;
    }

    size_t getNZ() override {
        return NZ;
    }

    /* Fills data from packed_data. */
    void fromPackedData(size_t num_elmnts) {
        data.clear();
        data.resize(num_elmnts);

        size_t elmt_idx = 0;
        std::bitset<NX+NY+NZ> elmt;
        size_t current_bit = 0;
        std::bitset<8> byte;
        for(size_t i = 0; i < getByteSize(); ++i) {
            byte = std::bitset<8>(static_cast<ulong>(packed_data[i]));
            for(size_t byte_idx = 0; byte_idx < byte.size(); ++byte_idx) {
                elmt[current_bit] = byte[byte_idx];
                current_bit = (current_bit + 1) % elmt.size();
                if(current_bit == 0) {
                    data[elmt_idx] = BitVec<NX,NY,NZ>(elmt);
                    elmt_idx++;
                    if(elmt_idx == data.size())
                        break;
                }
            }

            if(elmt_idx == data.size())
                break;
        }
    }

    /* Fills packed_data from data. */
    unsigned char* calcPackedData()
    {
        // delete old data
        if (packed_data != nullptr)
            delete [] packed_data;

        packed_data = new unsigned char[getByteSize()];
        std::bitset<8> byte;
        size_t current_byte = 0;
        size_t bit_idx = 0;
        // Pack all BitVec
        for(auto v: data) {
            // pack x-component
            for(size_t i = 0; i < NX; ++i) {
                byte[bit_idx] = v.x[i];
                bit_idx = (bit_idx + 1) % 8;
                if(bit_idx == 0) {
                    packed_data[current_byte] = static_cast<unsigned char>(byte.to_ulong());
                    ++current_byte;
                }
            }
            // pack y-component
            for(size_t i = 0; i < NY; ++i) {
                byte[bit_idx] = v.y[i];
                bit_idx = (bit_idx + 1) % 8;
                if(bit_idx == 0) {
                    packed_data[current_byte] = static_cast<unsigned char>(byte.to_ulong());
                    ++current_byte;
                }
            }
            // pack z-component
            for(size_t i = 0; i < NZ; ++i) {
                byte[bit_idx] = v.z[i];
                bit_idx = (bit_idx + 1) % 8;
                if(bit_idx == 0) {
                    packed_data[current_byte] = static_cast<unsigned char>(byte.to_ulong());
                    ++current_byte;
                }
            }
        }

        // add padding for last byte if necessary
        if(bit_idx != 0) {
            while(bit_idx != 0) {
                byte[bit_idx] = false;
                bit_idx = (bit_idx + 1) % 8;
            }
            packed_data[current_byte] = static_cast<unsigned char>(byte.to_ulong());
        }

        return packed_data;
    }

    std::vector<BitVec<NX,NY,NZ>> data;
    unsigned char* packed_data;
};

template <size_t N>
struct UniformBitVecArr : BitVecArray<N,N,N> {};

#endif //BITVECTOR_HPP
