using Microsoft.Xaml.Behaviors;
using System.Collections.Specialized;
using System.Windows.Controls;

namespace DMSS_GUI
{
    class AutoScrollBehavior : Behavior<ListBox>
    {
        protected override void OnAttached()
        {
            base.OnAttached();
            AssociatedObject.Loaded += (s, e) =>
            {
                var collection = AssociatedObject.Items;
                ((INotifyCollectionChanged)collection).CollectionChanged += (s2, e2) =>
                {
                    if (collection.Count > 0)
                        AssociatedObject.ScrollIntoView(collection[collection.Count - 1]);
                };
            };
        }
    }
}
