#include <iostream>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;


int main(int, char**) {
    mongocxx::instance inst{};
	//mongocxx::uri uri("mongodb:://localhost:27000");
	//mongocxx::client conn(uri); 
    mongocxx::client conn{mongocxx::uri{}};
    auto collection = conn["testdb"]["testcollection"];

    bsoncxx::builder::stream::document document{};

	//Using the stream builder interface, you can construct this document as follows:
	bsoncxx::document::value doc_value = document
	  << "name" << "MongoDB"
	  << "type" << "database"
	  << "count" << 1
	  << "versions" << bsoncxx::builder::stream::open_array
		<< "v3.2" << "v3.0" << "v2.6"
	  << close_array
	  << "info" << bsoncxx::builder::stream::open_document
		<< "x" << 203
		<< "y" << 102
	  << bsoncxx::builder::stream::close_document
      << bsoncxx::builder::stream::finalize;

	//This bsoncxx::document::value type is a read-only object owning its own memory. 
	//To use it, you must obtain a bsoncxx::document::view using the view() method:
	bsoncxx::document::view view = doc_value.view();
    collection.insert_one(view);

	//delete many 
	bsoncxx::stdx::optional<mongocxx::result::delete_result> resultD =
		collection.delete_many(
	  bsoncxx::builder::stream::document{} << "index" << open_document <<
		"$lte" << 100 << close_document << finalize);

	if(resultD) {
	  std::cout << "deleted total num: " << resultD->deleted_count() <<std::endl;
	}

	resultD = collection.delete_many(
      bsoncxx::builder::stream::document{} << "name" << open_document <<
        "$eq" << "MongoDB" << close_document << finalize);

	//insert many 
	std::vector<bsoncxx::document::value> documents;
	for ( int i = 49; i < 77; i++) {
		documents.push_back( bsoncxx::builder::stream::document{} << "index" << i <<finalize); 
	}
	collection.insert_many(documents);

	//delete one
    if (collection.delete_one ( bsoncxx::builder::stream::document{}<<"index" << 99 << finalize)) //bug here!! return true even if not found. 
        std::cout << "delete one done" << std::endl;
    else 
        std::cout << "delete none" << std::endl; 

	//update one
    collection.update_one (
        bsoncxx::builder::stream::document{} << "index" << 66 << finalize,
        bsoncxx::builder::stream::document{} << "$set" << 
        open_document << "index" << 666 << close_document << finalize);

	bsoncxx::stdx::optional<bsoncxx::document::value> resultU =
        collection.find_one (
		bsoncxx::builder::stream::document{} << "index" << 666  << finalize); 
    
	if (resultU ) {
        std::cout << "updated one: " << bsoncxx::to_json ( *resultU ) << std::endl;
    }


	//display all 
    auto cursor = collection.find({});
	
	for (auto&& doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << std::endl;
    }

	//query one
	bsoncxx::builder::stream::document query{}; 
	 bsoncxx::document::value  queryVal = query 
		 << "index" << 75  << finalize; 
	bsoncxx::document::view queryView = queryVal.view();

	bsoncxx::stdx::optional<bsoncxx::document::value> result =
		collection.find_one ( queryView ) ;
	if (result ) {
		std::cout << "found one: " << bsoncxx::to_json ( *result ) << std::endl; 
	}

	//query many 
	mongocxx::cursor cursor2 = collection.find(
		bsoncxx::builder::stream::document{} << "index" << open_document <<
		"$gt" << 50 << "$lte" << 55
	   	<< close_document << finalize);
	for(auto doc : cursor2) {
	  	std::cout << "found many: " << bsoncxx::to_json(doc) << std::endl;
	}
}

