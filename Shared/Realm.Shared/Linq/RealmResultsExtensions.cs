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

using System;
using System.Linq;
using System.Linq.Expressions;

namespace Realms
{
    /// <summary>
    /// Extensions for executing additional queries.
    /// </summary>
    public static class RealmResultsExtensions
    {
        /// <summary>
        /// Add links query.
        /// </summary>
        /// <typeparam name="T">Original type.</typeparam>
        /// <param name="query">Original query.</param>
        /// <param name="propertyPath">Link property name.</param>
        /// <param name="predicateOperator">Predicate operator.</param>
        /// <param name="value">Value to compare with.</param>
        /// <returns>Returns IQueryable.</returns>
        public static IQueryable<T> AddLinkQuery<T>(this IQueryable<T> query, string propertyPath, PredicateOperator predicateOperator, object value)
            where T : RealmObject
        {
            var realmResults = (RealmResults<T>)query;
            realmResults.AddLinkQueryInternal(propertyPath, predicateOperator, value);
            return query;
        }

        /// <summary>
        /// Add links query.
        /// </summary>
        /// <typeparam name="T">Original type.</typeparam>
        /// <typeparam name="TLink">Linked type.</typeparam>
        /// <param name="query">Original query.</param>
        /// <param name="property">Link property name.</param>
        /// <param name="predicateOperator">Query string to search.</param>
        /// <param name="value">Value string to search.</param>
        /// <returns>Returns IQueryable.</returns>
        public static IQueryable<T> AddLinkQuery<T, TLink>(this IQueryable<T> query, Expression<Func<T, TLink>> property, PredicateOperator predicateOperator, TLink value)
            where T : RealmObject
        {
            var realmResults = (RealmResults<T>)query;
            realmResults.AddLinkQueryInternal<TLink>(property, predicateOperator, value);
            return query;
        }
    }
}