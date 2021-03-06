#pragma once
#include <fc/io/raw_fwd.hpp>


namespace fc {


   /**
    *  This class is designed to offer in-place memory allocation of a string up to Length equal to
    *  sizeof(Storage).
    *
    *  The string will serialize the same way as std::string for variant and raw formats
    *  The string will sort according to the comparison operators defined for Storage, this enables effecient
    *  sorting.
    */
   template<typename Storage = std::pair<uint64_t,uint64_t> >
   class fixed_string {
      public:
         fixed_string(){}
         fixed_string( const fixed_string& c ):data(c.data){}

         fixed_string( const std::string& str ) {
            if( str.size() <= sizeof(data) )
               memcpy( (char*)&data, str.c_str(), str.size() );
            else {
               memcpy( (char*)&data, str.c_str(), sizeof(data) );
            }
         }
         fixed_string( const char* str ) {
            auto l = strlen(str);
            if( l <= sizeof(data) )
               memcpy( (char*)&data, str, l );
            else {
               memcpy( (char*)&data, str, sizeof(data) );
            }
         }

         operator std::string()const {
            const char* self = (const char*)&data;
            return std::string( self, self + size() );
         }

         uint32_t size()const {
            if( *(((const char*)&data)+sizeof(data) - 1) )
               return sizeof(data);
            return strnlen( (const char*)&data, sizeof(data) );
         }
         uint32_t length()const { return size(); }

         fixed_string& operator=( const fixed_string& str ) {
            data = str.data;
            return *this;
         }
         fixed_string& operator=( const char* str ) {
            return *this = fixed_string(str);
         }

         fixed_string& operator=( const std::string& str ) {
            if( str.size() <= sizeof(data) ) {
               data = Storage();
               memcpy( (char*)&data, str.c_str(), str.size() );
            }
            else {
               memcpy( (char*)&data, str.c_str(), sizeof(data) );
            }
            return *this;
         }

         friend std::string operator + ( const fixed_string& a, const std::string& b ) {
            return std::string(a) + b;
         }
         friend std::string operator + ( const std::string& a, const fixed_string& b ) {
            return a + std::string(b);
         }

         friend bool operator < ( const fixed_string& a, const fixed_string& b ) {
            return a.data < b.data;
         }
         friend bool operator <= ( const fixed_string& a, const fixed_string& b ) {
            return a.data <= b.data;
         }
         friend bool operator > ( const fixed_string& a, const fixed_string& b ) {
            return a.data > b.data;
         }
         friend bool operator >= ( const fixed_string& a, const fixed_string& b ) {
            return a.data >= b.data;
         }
         friend bool operator == ( const fixed_string& a, const fixed_string& b ) {
            return a.data == b.data;
         }
         friend bool operator != ( const fixed_string& a, const fixed_string& b ) {
            return a.data != b.data;
         }
      //private:
         Storage data;
   };

  namespace raw
  {
    template<typename Stream, typename Storage>
    inline void pack( Stream& s, const fc::fixed_string<Storage>& u, uint32_t _max_depth=FC_PACK_MAX_DEPTH ) {
       FC_ASSERT( _max_depth > 0 );
       unsigned_int size = u.size();
       pack( s, size, _max_depth - 1 );
       s.write( (const char*)&u.data, size );
    }

    template<typename Stream, typename Storage>
    inline void unpack( Stream& s, fc::fixed_string<Storage>& u, uint32_t _max_depth=FC_PACK_MAX_DEPTH ) {
       FC_ASSERT( _max_depth > 0 );
       unsigned_int size;
       fc::raw::unpack( s, size, _max_depth - 1 );
       if( size.value > 0 ) {
          if( size.value > sizeof(Storage) ) {
             s.read( (char*)&u.data, sizeof(Storage) );
             char buf[1024];
             size_t left = size.value - sizeof(Storage);
             while( left >= 1024 )
             {
                s.read( buf, 1024 );
                left -= 1024;
             }
             s.read( buf, left );
          } else {
             s.read( (char*)&u.data, size.value );
          }
       }
    }
  }
}

#include <fc/variant.hpp>
namespace fc {
   template<typename Storage>
   void to_variant( const fixed_string<Storage>& s, variant& v, uint32_t max_depth = 1 ) {
      v = std::string(s);
   }

   template<typename Storage>
   void from_variant( const variant& v, fixed_string<Storage>& s, uint32_t max_depth = 1 ) {
      s = v.as_string();
   }
}
