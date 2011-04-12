// Generated by the protocol buffer compiler.  DO NOT EDIT!

#ifndef PROTOBUF_xmlext_2eproto__INCLUDED
#define PROTOBUF_xmlext_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2002000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2002000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
#include "google/protobuf/descriptor.pb.h"

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_xmlext_2eproto();
void protobuf_AssignDesc_xmlext_2eproto();
void protobuf_ShutdownFile_xmlext_2eproto();

class xmloption;

enum xmltype {
  text = 1,
  duration = 2
};
bool xmltype_IsValid(int value);
const xmltype xmltype_MIN = text;
const xmltype xmltype_MAX = duration;

const ::google::protobuf::EnumDescriptor* xmltype_descriptor();
inline const ::std::string& xmltype_Name(xmltype value) {
  return ::google::protobuf::internal::NameOfEnum(
    xmltype_descriptor(), value);
}
inline bool xmltype_Parse(
    const ::std::string& name, xmltype* value) {
  return ::google::protobuf::internal::ParseNamedEnum<xmltype>(
    xmltype_descriptor(), name, value);
}
// ===================================================================

class xmloption : public ::google::protobuf::Message {
 public:
  xmloption();
  virtual ~xmloption();
  
  xmloption(const xmloption& from);
  
  inline xmloption& operator=(const xmloption& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const xmloption& default_instance();
  void Swap(xmloption* other);
  
  // implements Message ----------------------------------------------
  
  xmloption* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const xmloption& from);
  void MergeFrom(const xmloption& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const { _cached_size_ = size; }
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // required string path = 1;
  inline bool has_path() const;
  inline void clear_path();
  static const int kPathFieldNumber = 1;
  inline const ::std::string& path() const;
  inline void set_path(const ::std::string& value);
  inline void set_path(const char* value);
  inline void set_path(const char* value, size_t size);
  inline ::std::string* mutable_path();
  
  // optional .xmltype type = 2 [default = text];
  inline bool has_type() const;
  inline void clear_type();
  static const int kTypeFieldNumber = 2;
  inline xmltype type() const;
  inline void set_type(xmltype value);
  
 private:
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  mutable int _cached_size_;
  
  ::std::string* path_;
  static const ::std::string _default_path_;
  int type_;
  friend void  protobuf_AddDesc_xmlext_2eproto();
  friend void protobuf_AssignDesc_xmlext_2eproto();
  friend void protobuf_ShutdownFile_xmlext_2eproto();
  
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];
  
  // WHY DOES & HAVE LOWER PRECEDENCE THAN != !?
  inline bool _has_bit(int index) const {
    return (_has_bits_[index / 32] & (1u << (index % 32))) != 0;
  }
  inline void _set_bit(int index) {
    _has_bits_[index / 32] |= (1u << (index % 32));
  }
  inline void _clear_bit(int index) {
    _has_bits_[index / 32] &= ~(1u << (index % 32));
  }
  
  void InitAsDefaultInstance();
  static xmloption* default_instance_;
};
// ===================================================================


// ===================================================================

static const int kXmlFieldNumber = 50000;
extern ::google::protobuf::internal::ExtensionIdentifier< ::google::protobuf::FieldOptions,
    ::google::protobuf::internal::MessageTypeTraits< ::xmloption >, 11, false >
  xml;

// ===================================================================

// xmloption

// required string path = 1;
inline bool xmloption::has_path() const {
  return _has_bit(0);
}
inline void xmloption::clear_path() {
  if (path_ != &_default_path_) {
    path_->clear();
  }
  _clear_bit(0);
}
inline const ::std::string& xmloption::path() const {
  return *path_;
}
inline void xmloption::set_path(const ::std::string& value) {
  _set_bit(0);
  if (path_ == &_default_path_) {
    path_ = new ::std::string;
  }
  path_->assign(value);
}
inline void xmloption::set_path(const char* value) {
  _set_bit(0);
  if (path_ == &_default_path_) {
    path_ = new ::std::string;
  }
  path_->assign(value);
}
inline void xmloption::set_path(const char* value, size_t size) {
  _set_bit(0);
  if (path_ == &_default_path_) {
    path_ = new ::std::string;
  }
  path_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* xmloption::mutable_path() {
  _set_bit(0);
  if (path_ == &_default_path_) {
    path_ = new ::std::string;
  }
  return path_;
}

// optional .xmltype type = 2 [default = text];
inline bool xmloption::has_type() const {
  return _has_bit(1);
}
inline void xmloption::clear_type() {
  type_ = 1;
  _clear_bit(1);
}
inline xmltype xmloption::type() const {
  return static_cast< xmltype >(type_);
}
inline void xmloption::set_type(xmltype value) {
  GOOGLE_DCHECK(xmltype_IsValid(value));
  _set_bit(1);
  type_ = value;
}


#ifndef SWIG
namespace google {
namespace protobuf {

template <>
inline const EnumDescriptor* GetEnumDescriptor< xmltype>() {
  return xmltype_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

#endif  // PROTOBUF_xmlext_2eproto__INCLUDED
