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
    internal interface IQueryableCollection
    {
        QueryHandle CreateQuery();
    }

    internal class RealmResults<T> : RealmCollectionBase<T>, IOrderedQueryable<T>, IQueryableCollection
    {
        private readonly RealmResultsProvider _provider;
        private readonly bool _allRecords;
        private readonly ResultsHandle _handle;

        internal ResultsHandle ResultsHandle => (ResultsHandle)Handle.Value;

        public Type ElementType => typeof(T);

        public Expression Expression { get; } // null if _allRecords

        public IQueryProvider Provider => _provider;

        internal RealmResults(Realm realm, RealmResultsProvider realmResultsProvider, Expression expression, RealmObject.Metadata metadata, bool createdByAll) : base(realm, metadata)
        {
            _provider = realmResultsProvider;
            Expression = expression ?? Expression.Constant(this);
            _allRecords = createdByAll;
        }

        internal RealmResults(Realm realm, RealmObject.Metadata metadata, bool createdByAll)
            : this(realm, new RealmResultsProvider(realm, metadata), null, metadata, createdByAll)
        {
        }

        internal RealmResults(Realm realm, ResultsHandle handle, RealmObject.Metadata metadata)
            : this(realm, new RealmResultsProvider(realm, metadata), null, metadata, false)
        {
            _handle = handle;
        }

        public QueryHandle CreateQuery()
        {
            return ResultsHandle.CreateQuery();
        }

        internal override CollectionHandleBase CreateHandle()
        {
            if (_handle != null)
            {
                return _handle;
            }

            var expression = Expression;
            if (_allRecords)
            {
                if (_linkedQueries?.Count > 0)
                {
                    var results = new RealmResults<T>(this.Realm, Metadata, true);
                    var query = results.CreateQuery();
                    AddLinkQueries(query);
                    return Realm.MakeResultsForQuery(query, null);
                }
                else
                {
                    return Realm.MakeResultsForTable(Metadata);
                }
            }

            // do all the LINQ expression evaluation to build a query
            var qv = _provider.MakeVisitor();
            qv.Visit(expression);
            var queryHandle = qv.CoreQueryHandle; // grab out the built query definition
            AddLinkQueries(queryHandle);

            var sortHandle = qv.OptionalSortDescriptorBuilder;
            return Realm.MakeResultsForQuery(queryHandle, sortHandle);
        }

        private void AddLinkQueries(QueryHandle query)
        {
            if (_linkedQueries != null)
            {
                foreach (var linkedQueryInfo in _linkedQueries)
                {
                    var propertyPath = linkedQueryInfo.PropertyPath.Split('.');
                    var metadata = Metadata;
                    System.Collections.Generic.Stack<IntPtr> propertyIndexes = new System.Collections.Generic.Stack<IntPtr>();
                    var i = 0;
                    Realms.Schema.Property property = new Realms.Schema.Property();
                    foreach (var propertyName in propertyPath)
                    {
                        if (!metadata.Schema.TryFindProperty(propertyName, out property))
                        {
                            throw new InvalidOperationException($"{propertyName} not found");
                        }

                        propertyIndexes.Push(metadata.Table.GetColumnIndex(propertyName));

                        if (i < propertyPath.Length - 1)
                        {
                            metadata = Realm.Metadata[property.ObjectType];
                        }
                        i++;
                    }
                    var lastPropertyId = propertyIndexes.Pop();
                    if (linkedQueryInfo.Value == null)
                    {
                        if (linkedQueryInfo.PredicateOperator == PredicateOperator.Equal)
                        {
                            query.CreateLinkQueryNull(propertyIndexes.ToArray(), lastPropertyId);
                        }
                        else if (linkedQueryInfo.PredicateOperator == PredicateOperator.NotEqual)
                        {
                            query.CreateLinkQueryNotNull(propertyIndexes.ToArray(), lastPropertyId);
                        }
                        else
                        {
                            throw new InvalidOperationException($"Can't perform {linkedQueryInfo.PredicateOperator} operaion on null");
                        }

                    }
                    else if (property.Type == Realms.Schema.PropertyType.String)
                    {
                        query.CreateLinkQueryString(propertyIndexes.ToArray(), lastPropertyId, linkedQueryInfo.PredicateOperator, (string)linkedQueryInfo.Value, false);
                    }
                    else if (property.Type == Realms.Schema.PropertyType.Int)
                    {
                        query.CreateLinkQueryInt(propertyIndexes.ToArray(), lastPropertyId, linkedQueryInfo.PredicateOperator, (int)linkedQueryInfo.Value);
                    }
                    else if (property.Type == Realms.Schema.PropertyType.Bool)
                    {
                        query.CreateLinkQueryBool(propertyIndexes.ToArray(), lastPropertyId, linkedQueryInfo.PredicateOperator, (bool)linkedQueryInfo.Value);
                    }
                    else if (property.Type == Realms.Schema.PropertyType.Float)
                    {
                        query.CreateLinkQueryFloat(propertyIndexes.ToArray(), lastPropertyId, linkedQueryInfo.PredicateOperator, (float)linkedQueryInfo.Value);
                    }
                    else if (property.Type == Realms.Schema.PropertyType.Double)
                    {
                        query.CreateLinkQueryDouble(propertyIndexes.ToArray(), lastPropertyId, linkedQueryInfo.PredicateOperator, (double)linkedQueryInfo.Value);
                    }
                    else if (property.Type == Realms.Schema.PropertyType.Date)
                    {
                        query.CreateLinkQueryDate(propertyIndexes.ToArray(), lastPropertyId, linkedQueryInfo.PredicateOperator, (DateTimeOffset)linkedQueryInfo.Value);
                    }
                    else
                    {
                        throw new NotImplementedException($"Type {linkedQueryInfo.Value.GetType()} not supported");
                    }
                }
            }
        }

        private class LinkQuery
        {
            public string PropertyPath { get; set; }

            public PredicateOperator PredicateOperator { get; set; }

            public object Value { get; set; }
        }

        private System.Collections.Generic.List<LinkQuery> _linkedQueries;

        public void AddLinkQueryInternal<TLink>(Expression<Func<T, TLink>> property, PredicateOperator predicateOperator, TLink value)
        {
            _linkedQueries = _linkedQueries ?? new System.Collections.Generic.List<LinkQuery>();
            var name = property.Parameters[0].Name;
            _linkedQueries.Add(new LinkQuery()
            {
                PropertyPath = property.ToString().Replace($"{name} => {name}.", string.Empty),
                PredicateOperator = predicateOperator,
                Value = value,
            });
        }

        public void AddLinkQueryInternal(string propertyPath, PredicateOperator predicateOperator, object value)
        {
            _linkedQueries = _linkedQueries ?? new System.Collections.Generic.List<LinkQuery>();
            _linkedQueries.Add(new LinkQuery()
            {
                PropertyPath = propertyPath,
                PredicateOperator = predicateOperator,
                Value = value,
            });
        }
    }
}