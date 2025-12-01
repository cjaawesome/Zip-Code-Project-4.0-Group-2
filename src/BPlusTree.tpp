#include "BPlusTree.h"

template <typename keyType, typename valueType>
BPlusTree<keyType, valueType>::BPlusTree(){

}

template <typename keyType, typename valueType>
BPlusTree<keyType, valueType>::~BPlusTree(){
    
}

template <typename keyType, typename valueType>
bool BPlusTree<keyType, valueType>::insert(const keyType& key, const valueType& value){
    
}
template <typename keyType, typename valueType>
bool BPlusTree<keyType, valueType>::search(const keyType& key, valueType& outValue) const{
    
}
template <typename keyType, typename valueType>
bool BPlusTree<keyType, valueType>::open(const std::string& filename){

}

template <typename keyType, typename valueType>
void BPlusTree<keyType, valueType>::close(){

}

template <typename keyType, typename valueType>
bool BPlusTree<keyType, valueType>::isOpen() const{
    return isFileOpen;
}