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
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;
using Realms;

// NOTE some of the following data comes from Tim's data used in the Browser screenshot in the Mac app store
// unlike the Cocoa definitions, we use Pascal casing for properties
namespace Tests.Database
{
    [TestFixture, Preserve(AllMembers = true)]
    public class LinkQueryTests
    {
        protected Realm realm;

        [SetUp]
        public void SetUp()
        {
            Realm.DeleteRealm(RealmConfiguration.DefaultConfiguration);
            realm = Realm.GetInstance();

            // we don't keep any variables pointing to these as they are all added to Realm
            using (var trans = realm.BeginWrite())
            {
                var o1 = realm.Add(new Owner { Name = "Tim" });

                var d1 = realm.Add(new Dog
                {
                    Name = "Bilbo Fleabaggins",
                    Color = "Black"
                });

                o1.TopDog = d1;  // set a one-one relationship
                o1.Dogs.Add(d1);

                var d2 = realm.Add(new Dog
                {
                    Name = "Earl Yippington III",
                    Color = "White"
                });

                o1.Dogs.Add(d2);

                // lonely people and dogs
                realm.Add(new Owner
                {
                    Name = "Dani" // the dog-less
                });

                realm.Add(new Dog // will remain unassigned
                {
                    Name = "Maggie Mongrel",
                    Color = "Grey"
                });

                trans.Commit();
            }
        }

        [TearDown]
        public void TearDown()
        {
            realm.Dispose();
            Realm.DeleteRealm(realm.Config);
        }

        [Test]
        public void TimHasABlackTopDog()
        {
            var tim = realm.All<Owner>().AddLinkQuery(x => x.TopDog.Color, PredicateOperator.Equal, "Black").FirstOrDefault();
            Assert.That(tim.Name, Is.EqualTo("Tim"));
        }

        [Test]
        public void TimHasNoWhiteTopDog()
        {
            var tim = realm.All<Owner>().AddLinkQuery(x => x.TopDog.Color, PredicateOperator.Equal, "White").FirstOrDefault();
            Assert.That(tim, Is.Null);
        }

        [Test]
        public void TimHasWhiteDog()
        {
            var tim = realm.All<Owner>().AddLinkQuery($"{nameof(Owner.Dogs)}.{nameof(Dog.Color)}", PredicateOperator.Equal, "White").FirstOrDefault();
            Assert.That(tim.Name, Is.EqualTo("Tim"));
        }

        [Test]
        public void QueryOnBacklink()
        {
            // backlink queries are not supported yet (waiting for Java implementation to port :))
            try
            {
                var dog = realm.All<Dog>().AddLinkQuery($"Owners.Name", PredicateOperator.Equal, "Tim").ToList();
            }
            catch (InvalidOperationException e)
            {
                Assert.That(e.Message, Does.Contain("Owners is a backlink, queries are not supported yet"));
                return;
            }
            Assert.Fail("Exception should be ");
        }

        [Test]
        public void LinkQueryTwoLevelsDeep()
        {
            realm.Write(() =>
            {
                var category1 = new ItemCategory() { Id = "1" };
                var category2 = new ItemCategory() { Id = "2" };
                realm.Add(category1);
                realm.Add(category2);
                var model1 = new ItemModel() { Id = "11", ItemCategory = category1 };
                var model2 = new ItemModel() { Id = "12", ItemCategory = category2 };
                realm.Add(model1);
                realm.Add(model2);
                realm.Add(new Item() { Id = "21", ItemModel = model1 });
            });
            var item = realm.All<Item>().AddLinkQuery(x => x.ItemModel.ItemCategory.Id, PredicateOperator.Equal, "1").ToList();
            Assert.That(item[0].Id, Is.EqualTo("21"));
        }

        public class Item : RealmObject
        {
            public Item()
            {
            }

            [PrimaryKey]
            public string Id { get; set; }

            public string TestPropertyBeforeItemModel { get; set; }

            public ItemModel ItemModel { get; set; }
        }

        public class ItemModel : RealmObject
        {
            [PrimaryKey]
            public string Id { get; set; }

            public ItemCategory ItemCategory { get; set; }

            public ItemCategory Category { get; set; }
        }

        public class ItemCategory : RealmObject
        {
            [PrimaryKey]
            public string Id { get; set; }

            public string Name { get; set; }
        }
    }
}