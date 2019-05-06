//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public class DictionaryPatcher<K, V> implements Ice.ReadValueCallback
{
    public DictionaryPatcher(java.util.Map<K, V> dict, Class<V> cls, K key)
    {
        _dict = dict;
        _cls = cls;
        _key = key;
    }

    public void valueReady(Ice.Object v)
    {
        if(v != null)
        {
            //
            // Raise ClassCastException if the element doesn't match the expected type.
            //
            if(!_cls.isInstance(v))
            {
                throw new ClassCastException("expected element of type " + _cls.getName() + " but received " +
                                             v.getClass().getName());
            }
        }

        _dict.put(_key, _cls.cast(v));
    }

    private java.util.Map<K, V> _dict;
    private Class<V> _cls;
    private K _key;
}
