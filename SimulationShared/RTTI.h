#pragma once

#include <string>
#include <cstdint>

namespace Library_Desktop
{
	typedef std::uint64_t ClassType;
	class RTTI
	{
	public:
		virtual ~RTTI() = default;
		
		virtual ClassType TypeIdInstance() const = 0;
		
		virtual RTTI* QueryInterface(const ClassType) const
		{
			return nullptr;
		}

		virtual bool Is(ClassType) const
		{
			return false;
		}

		virtual bool Is(const std::string&) const
		{
			return false;
		}
		template <typename T>
		T* As() const
		{
			if (Is(T::TypeIdClass()))
			{
				return (T*)this;
			}

			return nullptr;
		}

		virtual std::string ToString() const
		{
			return "RTTI";
		}

		virtual bool Equals(const RTTI* rhs) const
		{
			return this == rhs;
		}

		static bool IsA(ClassType id)
		{
			(id);
			return false;
		}
	};
#define INVALID_CLASS_TYPE	((ClassType)0)
#define RTTI_DECLARATIONS(Type, ParentType)																	\
	public:																									\
			typedef ParentType Parent;																		\
			static std::string TypeName() { return std::string(#Type); }									\
			static ClassType TypeIdClass() { return sRunTimeTypeId; }										\
			static bool IsA(ClassType id)																	\
			{																								\
				if(sRunTimeTypeId == id)																	\
				{																							\
					return true;																			\
				}																							\
				return ParentType::IsA(id);																	\
			}																								\
			virtual ClassType TypeIdInstance() const override { return Type::TypeIdClass(); }				\
			virtual Library_Desktop::RTTI* QueryInterface(const ClassType id) const override				\
            {																								\
                if (id == sRunTimeTypeId)																	\
					{ return (RTTI*)this; }																	\
                else																						\
					{ return Parent::QueryInterface(id); }													\
            }																								\
			virtual bool Is(ClassType id) const override													\
			{																								\
				if (id == sRunTimeTypeId)																	\
					{ return true; }																		\
				else																						\
					{ return Parent::Is(id); }																\
			}																								\
			virtual bool Is(const std::string& name) const override											\
			{																								\
				if (name == TypeName())																		\
					{ return true; }																		\
				else																						\
					{ return Parent::Is(name); }															\
			}																								\
			private:																						\
				static ClassType sRunTimeTypeId;

#define RTTI_DEFINITIONS(Type) ClassType Type::sRunTimeTypeId = reinterpret_cast<ClassType>(&Type::sRunTimeTypeId);
}




/*==================================== Potential Function Implementations ====================================

		template <typename T>
		bool Is(T*& = nullptr)
		{
			return false;
		}
		
#define RTTI_DECLARATIONS(Type, ParentType)																	\
				. 
				.
				.

			template <typename T>																			\
			bool Is(T *& casted = nullptr) const															\
			{																								\
				if((*casted) = As<T>())																		\
					{ return true; }																		\
				else																						\
					{ return false; }																		\
			}																								\
				.
				.
				.

				

			// Test Self by class
			Assert::IsFalse(rtti->Is<T>());

			// Test Unknown by class
			Assert::IsFalse(rtti->Is<Foo>());

			// Test RTTI Is<T> call in condition
			T * asType = nullptr;
			if (rtti->Is<T>(&asType))
			{
				Assert::IsNotNull(asType);
			}
			else
			{
				Assert::Fail();
			}
*/