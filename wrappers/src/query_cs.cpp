////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////
 
#include <realm.hpp>
#include <realm/lang_bind_helper.hpp>
#include "marshalling.hpp"
#include "error_handling.hpp"
#include "realm_export_decls.hpp"
#include "object-store/src/shared_realm.hpp"
#include "object-store/src/schema.hpp"
#include "timestamp_helpers.hpp"
#include "object-store/src/results.hpp"
#include "marshalable_sort_clause.hpp"
#include "object_accessor.hpp"


using namespace realm;
using namespace realm::binding;

extern "C" {

REALM_EXPORT void query_destroy(Query* query_ptr)
{
    delete(query_ptr);
}

REALM_EXPORT Object* query_find(Query* query_ptr, size_t begin_at_table_row, SharedRealm* realm, NativeException::Marshallable& ex)
{
    return handle_errors(ex, [&]() -> Object* {
        if (begin_at_table_row >= query_ptr->get_table()->size())
            return nullptr;

        size_t row_ndx = query_ptr->find(begin_at_table_row);

        if (row_ndx == not_found)
            return nullptr;

        const std::string object_name(ObjectStore::object_type_for_table_name(query_ptr->get_table()->get_name()));
        auto& object_schema = *realm->get()->schema().find(object_name);
        return new Object(*realm, object_schema, Row((*query_ptr->get_table())[row_ndx]));
    });
}
    
REALM_EXPORT Object* query_find_next(Query* query_ptr, const Object& after_object, NativeException::Marshallable& ex)
{
    auto realm = after_object.realm();
    return query_find(query_ptr, after_object.row().get_index() + 1, &realm, ex);
}
    
REALM_EXPORT size_t query_count(Query * query_ptr, NativeException::Marshallable& ex)
{
    return handle_errors(ex, [&]() {
        return query_ptr->count();
    });
}


//convert from columnName to columnIndex returns -1 if the string is not a column name
//assuming that the get_table() does not return anything that must be deleted
REALM_EXPORT size_t query_get_column_index(Query* query_ptr, uint16_t* column_name, size_t column_name_len, NativeException::Marshallable& ex)
{
    return handle_errors(ex, [&]() {
        Utf16StringAccessor str(column_name, column_name_len);
        return query_ptr->get_table()->get_column_index(str);
    });
}

REALM_EXPORT void query_not(Query * query_ptr, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->Not();
    });
}

REALM_EXPORT void query_group_begin(Query * query_ptr, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->group();
    });
}

REALM_EXPORT void query_group_end(Query * query_ptr, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->end_group();
    });
}

REALM_EXPORT void query_or(Query * query_ptr, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->Or();
    });
}

REALM_EXPORT void query_string_contains(Query* query_ptr, size_t columnIndex, uint16_t* value, size_t value_len, bool case_sensitive, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        Utf16StringAccessor str(value, value_len);
        query_ptr->contains(columnIndex, str, case_sensitive);
    });
}

REALM_EXPORT void query_string_starts_with(Query* query_ptr, size_t columnIndex, uint16_t* value, size_t value_len, bool case_sensitive, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        Utf16StringAccessor str(value, value_len);
        query_ptr->begins_with(columnIndex, str, case_sensitive);
    });
}

REALM_EXPORT void query_string_ends_with(Query* query_ptr, size_t columnIndex, uint16_t* value, size_t value_len, bool case_sensitive, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        Utf16StringAccessor str(value, value_len);
        query_ptr->ends_with(columnIndex, str, case_sensitive);
    });
}
    
REALM_EXPORT void query_string_equal(Query * query_ptr, size_t columnIndex, uint16_t* value, size_t value_len, bool case_sensitive, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        Utf16StringAccessor str(value, value_len);
        query_ptr->equal(columnIndex, str, case_sensitive);
    });
}

REALM_EXPORT void query_string_not_equal(Query * query_ptr, size_t columnIndex, uint16_t* value, size_t value_len, bool case_sensitive, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        Utf16StringAccessor str(value, value_len);
        query_ptr->not_equal(columnIndex, str, case_sensitive);
    });
}
    
REALM_EXPORT void query_string_like(Query * query_ptr, size_t columnIndex, uint16_t* value, size_t value_len, bool case_sensitive, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        Utf16StringAccessor str(value, value_len);
        query_ptr->like(columnIndex, str, case_sensitive);
    });
}

REALM_EXPORT void query_bool_equal(Query * query_ptr, size_t columnIndex, size_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->equal(columnIndex, size_t_to_bool(value));
    });
}

REALM_EXPORT void query_bool_not_equal(Query * query_ptr, size_t columnIndex, size_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->not_equal(columnIndex, size_t_to_bool(value));
    });
}

REALM_EXPORT void query_int_equal(Query * query_ptr, size_t columnIndex, size_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->equal(columnIndex, static_cast<int>(value));
    });
}

REALM_EXPORT void query_int_not_equal(Query * query_ptr, size_t columnIndex, size_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->not_equal(columnIndex, static_cast<int>(value));
    });
}

REALM_EXPORT void query_int_less(Query * query_ptr, size_t columnIndex, size_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->less(columnIndex, static_cast<int>(value));
    });
}

REALM_EXPORT void query_int_less_equal(Query * query_ptr, size_t columnIndex, size_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->less_equal(columnIndex, static_cast<int>(value));
    });
}

REALM_EXPORT void query_int_greater(Query * query_ptr, size_t columnIndex, size_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->greater(columnIndex, static_cast<int>(value));
    });
}

REALM_EXPORT void query_int_greater_equal(Query * query_ptr, size_t columnIndex, size_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->greater_equal(columnIndex, static_cast<int>(value));
    });
}

REALM_EXPORT void query_long_equal(Query * query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->equal(columnIndex, value);
    });
}

REALM_EXPORT void query_long_not_equal(Query * query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->not_equal(columnIndex, value);
    });
}

REALM_EXPORT void query_long_less(Query * query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->less(columnIndex, value);
    });
}

REALM_EXPORT void query_long_less_equal(Query * query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->less_equal(columnIndex, value);
    });
}

REALM_EXPORT void query_long_greater(Query * query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->greater(columnIndex, value);
    });
}

REALM_EXPORT void query_long_greater_equal(Query * query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->greater_equal(columnIndex, value);
    });
}
    
    REALM_EXPORT void query_float_equal(Query * query_ptr, size_t columnIndex, float value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->equal(columnIndex, static_cast<float>(value));
    });
}

REALM_EXPORT void query_float_not_equal(Query * query_ptr, size_t columnIndex, float value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->not_equal(columnIndex, static_cast<float>(value));
    });
}

REALM_EXPORT void query_float_less(Query * query_ptr, size_t columnIndex, float value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->less(columnIndex, static_cast<float>(value));
    });
}

REALM_EXPORT void query_float_less_equal(Query * query_ptr, size_t columnIndex, float value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->less_equal(columnIndex, static_cast<float>(value));
    });
}

REALM_EXPORT void query_float_greater(Query * query_ptr, size_t columnIndex, float value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->greater(columnIndex, static_cast<float>(value));
    });
}

REALM_EXPORT void query_float_greater_equal(Query * query_ptr, size_t columnIndex, float value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->greater_equal(columnIndex, static_cast<float>(value));
    });
}

REALM_EXPORT void query_double_equal(Query * query_ptr, size_t columnIndex, double value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->equal(columnIndex, static_cast<double>(value));
    });
}

REALM_EXPORT void query_double_not_equal(Query * query_ptr, size_t columnIndex, double value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->not_equal(columnIndex, static_cast<double>(value));
    });
}

REALM_EXPORT void query_double_less(Query * query_ptr, size_t columnIndex, double value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->less(columnIndex, static_cast<double>(value));
    });
}

REALM_EXPORT void query_double_less_equal(Query * query_ptr, size_t columnIndex, double value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->less_equal(columnIndex, static_cast<double>(value));
    });
}

REALM_EXPORT void query_double_greater(Query * query_ptr, size_t columnIndex, double value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->greater(columnIndex, static_cast<double>(value));
    });
}

REALM_EXPORT void query_double_greater_equal(Query * query_ptr, size_t columnIndex, double value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->greater_equal(columnIndex, static_cast<double>(value));
    });
}

REALM_EXPORT void query_timestamp_ticks_equal(Query* query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->equal(columnIndex, from_ticks(value));
    });
}

REALM_EXPORT void query_timestamp_ticks_not_equal(Query* query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->not_equal(columnIndex, from_ticks(value));
    });
}

REALM_EXPORT void query_timestamp_ticks_less(Query* query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->less(columnIndex, from_ticks(value));
    });
}

REALM_EXPORT void query_timestamp_ticks_less_equal(Query* query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->less_equal(columnIndex, from_ticks(value));
    });
}

REALM_EXPORT void query_timestamp_ticks_greater(Query* query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->greater(columnIndex, from_ticks(value));
    });
}

REALM_EXPORT void query_timestamp_ticks_greater_equal(Query* query_ptr, size_t columnIndex, int64_t value, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->greater_equal(columnIndex, from_ticks(value));
    });
}

REALM_EXPORT void query_binary_equal(Query* query_ptr, size_t columnIndex, char* buffer, size_t buffer_length, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->equal(columnIndex, BinaryData(buffer, buffer_length));
    });
}

REALM_EXPORT void query_binary_not_equal(Query* query_ptr, size_t columnIndex, char* buffer, size_t buffer_length, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->not_equal(columnIndex, BinaryData(buffer, buffer_length));
    });
}
    
REALM_EXPORT void query_object_equal(Query* query_ptr, size_t columnIndex, Object& object, NativeException::Marshallable& ex)
{
    handle_errors(ex, [&]() {
        query_ptr->links_to(columnIndex, object.row());
    });
}
    
REALM_EXPORT void query_null_equal(Query* query_ptr, size_t columnIndex, NativeException::Marshallable& ex)   
{   
    handle_errors(ex, [&]() {   
        query_ptr->equal(columnIndex, null());    
    });   
}   
    
REALM_EXPORT void query_null_not_equal(Query* query_ptr, size_t columnIndex, NativeException::Marshallable& ex)   
{   
    handle_errors(ex, [&]() {
        query_ptr->not_equal(columnIndex, null());
    });   
}

typedef enum
{
	None,
	Equal,
	NotEqual,
	LessThan,
	LessThanOrEqual,
	GreaterThan,
	GreaterThanOrEqual,
	BeginsWith,
	EndsWith,
	Contains
} PredicateOperator;


Table* link_table(Query* query_ptr, size_t link_column_indexes[], size_t link_column_indexes_length) {
	TableRef table = query_ptr->get_table();
	Table* t = table.get();
	for (int i = 0; i < link_column_indexes_length; ++i) {
		t->link(link_column_indexes[i]);
	}
	return t;
}

REALM_EXPORT void query_set_table_link(Query* query_ptr, size_t link_column_indexes[], size_t link_column_indexes_length, NativeException::Marshallable& ex)
{
	return handle_errors(ex, [&]() {
		link_table(query_ptr, link_column_indexes, link_column_indexes_length);
	});
}

REALM_EXPORT void query_link_add_string_comparison(Query * query_ptr, size_t* link_column_indexes, size_t link_column_indexes_length, size_t columnIndex, PredicateOperator predicateOperator, uint16_t* value, size_t value_len, bool case_sensitive, NativeException::Marshallable& ex)
{
	return handle_errors(ex, [&]() {
		//Table* table = link_table(query_ptr, link_column_indexes, link_column_indexes_length);
		TableRef table = query_ptr->get_table();
		Table* t = table.get();
		for (int i = 0; i < link_column_indexes_length; ++i) {
			t->link(link_column_indexes[i]);
		}


		Utf16StringAccessor str(value, value_len);

		auto column = table->column<String>(columnIndex);
		Query linkquery;
		if (predicateOperator == PredicateOperator::Equal) 
		{
			linkquery = column.equal(str, case_sensitive);
		}
		else if (predicateOperator == PredicateOperator::BeginsWith) 
		{
			linkquery = column.begins_with(str, case_sensitive);
		}
		else if (predicateOperator == PredicateOperator::NotEqual)
		{
			linkquery = column.not_equal(str, case_sensitive);
		}
		else if (predicateOperator == PredicateOperator::Contains)
		{
			linkquery = column.contains(str, case_sensitive);
		}
		else 
		{
			throw std::exception();
		}
		query_ptr->and_query(linkquery);

		return;
	});
}

REALM_EXPORT void query_link_add_int_comparison(Query * query_ptr, size_t* link_column_indexes, size_t link_column_indexes_length, size_t columnIndex, PredicateOperator predicateOperator, size_t value, NativeException::Marshallable& ex)
{
	return handle_errors(ex, [&]() {
		Table* table = link_table(query_ptr, link_column_indexes, link_column_indexes_length);

		auto column = table->column<Int>(columnIndex);
		Query linkquery;
		auto castedValue = static_cast<Int>(value);
		if (predicateOperator == PredicateOperator::Equal)
		{
			linkquery = column == castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThan)
		{
			linkquery = column > castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThanOrEqual)
		{
			linkquery = column >= castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThanOrEqual)
		{
			linkquery = column >= castedValue;
		}
		else if (predicateOperator == PredicateOperator::LessThanOrEqual)
		{
			linkquery = column <= castedValue;
		}
		else if (predicateOperator == PredicateOperator::LessThan)
		{
			linkquery = column < castedValue;
		}
		else if (predicateOperator == PredicateOperator::NotEqual)
		{
			linkquery = column != castedValue;
		}
		else
		{
			throw std::exception();
		}

		query_ptr->and_query(linkquery);

		return;
	});
}

REALM_EXPORT void query_link_add_long_comparison(Query * query_ptr, size_t* link_column_indexes, size_t link_column_indexes_length, size_t columnIndex, PredicateOperator predicateOperator, int64_t value, NativeException::Marshallable& ex)
{
	return handle_errors(ex, [&]() {
		Table* table = link_table(query_ptr, link_column_indexes, link_column_indexes_length);

		auto column = table->column<int64_t>(columnIndex);
		Query linkquery;
		auto castedValue = static_cast<int64_t>(value);
		if (predicateOperator == PredicateOperator::Equal)
		{
			linkquery = column == castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThan)
		{
			linkquery = column > castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThanOrEqual)
		{
			linkquery = column >= castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThanOrEqual)
		{
			linkquery = column >= castedValue;
		}
		else if (predicateOperator == PredicateOperator::LessThanOrEqual)
		{
			linkquery = column <= castedValue;
		}
		else if (predicateOperator == PredicateOperator::LessThan)
		{
			linkquery = column < castedValue;
		}
		else if (predicateOperator == PredicateOperator::NotEqual)
		{
			linkquery = column != castedValue;
		}
		else
		{
			throw std::exception();
		}

		query_ptr->and_query(linkquery);

		return;
	});
}

REALM_EXPORT void query_link_add_double_comparison(Query * query_ptr, size_t* link_column_indexes, size_t link_column_indexes_length, size_t columnIndex, PredicateOperator predicateOperator, double value, NativeException::Marshallable& ex)
{
	return handle_errors(ex, [&]() {
		Table* table = link_table(query_ptr, link_column_indexes, link_column_indexes_length);

		auto column = table->column<double>(columnIndex);
		Query linkquery;
		auto castedValue = static_cast<double>(value);
		if (predicateOperator == PredicateOperator::Equal)
		{
			linkquery = column == castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThan)
		{
			linkquery = column > castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThanOrEqual)
		{
			linkquery = column >= castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThanOrEqual)
		{
			linkquery = column >= castedValue;
		}
		else if (predicateOperator == PredicateOperator::LessThanOrEqual)
		{
			linkquery = column <= castedValue;
		}
		else if (predicateOperator == PredicateOperator::LessThan)
		{
			linkquery = column < castedValue;
		}
		else if (predicateOperator == PredicateOperator::NotEqual)
		{
			linkquery = column != castedValue;
		}
		else
		{
			throw std::exception();
		}

		query_ptr->and_query(linkquery);

		return;
	});
}

REALM_EXPORT void query_link_add_null_equal(Query * query_ptr, size_t* link_column_indexes, size_t link_column_indexes_length, size_t columnIndex, NativeException::Marshallable& ex)
{
	return handle_errors(ex, [&]() {
		Table* table = link_table(query_ptr, link_column_indexes, link_column_indexes_length);

		query_ptr->equal(columnIndex, null());

		return;
	});
}

REALM_EXPORT void query_link_add_null_not_equal(Query * query_ptr, size_t* link_column_indexes, size_t link_column_indexes_length, size_t columnIndex, NativeException::Marshallable& ex)
{
	return handle_errors(ex, [&]() {
		Table* table = link_table(query_ptr, link_column_indexes, link_column_indexes_length);

		query_ptr->not_equal(columnIndex, null());

		return;
	});
}

REALM_EXPORT void query_link_add_float_comparison(Query * query_ptr, size_t* link_column_indexes, size_t link_column_indexes_length, size_t columnIndex, PredicateOperator predicateOperator, float value, NativeException::Marshallable& ex)
{
	return handle_errors(ex, [&]() {
		Table* table = link_table(query_ptr, link_column_indexes, link_column_indexes_length);

		auto column = table->column<float>(columnIndex);
		Query linkquery;
		auto castedValue = static_cast<float>(value);
		if (predicateOperator == PredicateOperator::Equal)
		{
			linkquery = column == castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThan)
		{
			linkquery = column > castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThanOrEqual)
		{
			linkquery = column >= castedValue;
		}
		else if (predicateOperator == PredicateOperator::GreaterThanOrEqual)
		{
			linkquery = column >= castedValue;
		}
		else if (predicateOperator == PredicateOperator::LessThanOrEqual)
		{
			linkquery = column <= castedValue;
		}
		else if (predicateOperator == PredicateOperator::LessThan)
		{
			linkquery = column < castedValue;
		}
		else if (predicateOperator == PredicateOperator::NotEqual)
		{
			linkquery = column != castedValue;
		}
		else
		{
			throw std::exception();
		}

		query_ptr->and_query(linkquery);

		return;
	});
}


REALM_EXPORT void query_link_add_bool_comparison(Query * query_ptr, size_t* link_column_indexes, size_t link_column_indexes_length, size_t columnIndex, PredicateOperator predicateOperator, size_t value, NativeException::Marshallable& ex)
{
	return handle_errors(ex, [&]() {
		Table* table = link_table(query_ptr, link_column_indexes, link_column_indexes_length);

		auto column = table->column<bool>(columnIndex);
		Query linkquery;
		auto castedValue = size_t_to_bool(value);
		if (predicateOperator == PredicateOperator::Equal)
		{
			linkquery = column == castedValue;
		}
		else if (predicateOperator == PredicateOperator::NotEqual)
		{
			linkquery = column != castedValue;
		}
		else
		{
			throw std::exception();
		}

		query_ptr->and_query(linkquery);

		return;
	});
}

REALM_EXPORT void query_and(Query * query_ptr, const Query & linkquery, NativeException::Marshallable& ex)
{
	handle_errors(ex, [&]() {
		query_ptr->and_query(linkquery);
	});
}

REALM_EXPORT Results* query_create_results(Query* query_ptr, SharedRealm* realm, NativeException::Marshallable& ex)
{
    return handle_errors(ex, [&]() {
        return new Results(*realm, *query_ptr);
    });
}

REALM_EXPORT Results* query_create_sorted_results(Query* query_ptr, SharedRealm* realm, Table* table_ptr, MarshalableSortClause* sort_clauses, size_t clause_count, size_t* flattened_property_indices, NativeException::Marshallable& ex)
{
    return handle_errors(ex, [&]() {
        std::vector<std::vector<size_t>> column_indices;
        std::vector<bool> ascending;

        const std::string object_name(ObjectStore::object_type_for_table_name(table_ptr->get_name()));
        auto& properties = realm->get()->schema().find(object_name)->persisted_properties;
        unflatten_sort_clauses(sort_clauses, clause_count, flattened_property_indices, column_indices, ascending, properties);

        auto sort_descriptor = SortDescriptor(*table_ptr, column_indices, ascending);
        return new Results(*realm, *query_ptr, sort_descriptor);
    });
}

}   // extern "C"
