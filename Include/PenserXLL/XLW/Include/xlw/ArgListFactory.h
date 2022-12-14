//
//
//                           ArgListFactory.h
//
//
#ifndef ARG_LIST_FACTORY_H
#define ARG_LIST_FACTORY_H

#include <xlw/ArgList.h>
#include <map>
#include <string>

namespace xlw {

    template<class T>
    class ArgListFactory;

    template<typename T>
    class ArgListFactory
    {
    public:
        static ArgListFactory<T>& FactoryInstance()
        {
	        static ArgListFactory<T> object;
	        return object;
        }
        typedef T* (*CreateTFunction)(const ArgumentList& );
        void RegisterClass(std::string ClassId, CreateTFunction);
        T* CreateT(ArgumentList args);
        ~ArgListFactory(){};

    private:
        std::map<std::string, CreateTFunction> TheCreatorFunctions;
        std::string KnownTypes;
        ArgListFactory(){}
        ArgListFactory(const ArgListFactory&){}
        ArgListFactory& operator=(const ArgListFactory&){ return *this;}

    };




    template<typename T>
    void ArgListFactory<T>::RegisterClass(std::string ClassId, CreateTFunction CreatorFunction)
    {
         MakeLowerCase(ClassId);
         TheCreatorFunctions.insert(std::pair<std::string,CreateTFunction>(ClassId,CreatorFunction));
         KnownTypes+=" "+ClassId;
    }

    template<typename T>
    T* ArgListFactory<T>::CreateT(ArgumentList args)
    {

        std::string Id = args.GetStringArgumentValue("name");


        if  (TheCreatorFunctions.find(Id) == TheCreatorFunctions.end())
        {
            throw(Id+" is an unknown class. Known types are "+KnownTypes);
        }

        return (TheCreatorFunctions.find(Id)->second)(args);
    }


    // easy access function
    template<class T>
    T* GetFromFactory(const ArgumentList& args)
    {
        return ArgListFactory<T>::FactoryInstance().CreateT(args);
    }

}

#endif
