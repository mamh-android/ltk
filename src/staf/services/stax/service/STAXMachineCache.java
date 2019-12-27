/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2007                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import com.ibm.staf.STAFUtil;

/**
 * Caches machines where STAX XML files reside with their file separators
 */
public class STAXMachineCache
{
    private static final int DEFAULT_CACHE_SIZE = 20;
    
    private static STAXMachineCache instance;

    private Map<CacheKey, MachineCacheEntry> cache;
    private int maxCacheSize;
    
    /**
     * Initializes the STAXMachineCache instance.
     */
    private STAXMachineCache()
    {
        maxCacheSize = DEFAULT_CACHE_SIZE;
        cache = Collections.synchronizedMap(
            new HashMap<CacheKey, MachineCacheEntry>());
    }
    
    /**
     * Gets the cache instance.
     * 
     * @return the STAXMachineCache.
     */
    public static STAXMachineCache get()
    {
        if (instance == null)
        {
            instance = new STAXMachineCache();
        }
        
        return instance;
    }
    
    /**
     * Gets a file separater for a machine from the cache.
     * 
     * @param machine the machine where the XML file resides.
     * @return the file separator or null if the machine is not in the cache.
     */
    public String getFileSep(String machine)
    {
        String fileSep = null;
        
        machine = STAXFileCache.formatEndpoint(machine);

        MachineCacheEntry item = cache.get(new CacheKey(machine));
        
        if (item != null)
        {
            fileSep = item.getFileSep();
            item.hit();
        }
        
        return fileSep;
    }
    
    /**
     * Gets the contents of the cache.
     * 
     * @return a list of cached items.
     */
    public List<MachineCacheEntry> getCacheContents()
    {
        List<MachineCacheEntry> contents = null;
        
        synchronized(cache)
        {
            contents = new LinkedList<MachineCacheEntry>(cache.values());
        }
        
        return contents;
    }
    
    /**
     * Clears the contents of the cache.
     * 
     * @return the number of files cleared.
     */
    public int purge()
    {
        int cleared = cache.size();
        cache.clear();
        
        return cleared;
    }
    
    /**
     * Checks the cache to see if it contains an entry for the machine
     * 
     * @param machine a machine
     * @return true if the item is cached, false if the item is not cached
     */
    public boolean checkCache(String machine)
    {
        machine = STAXFileCache.formatEndpoint(machine);
        
        MachineCacheEntry item = cache.get(new CacheKey(machine));
        
        if (item != null)
            return true;
        else
            return false;
    }
    
    /**
     * Adds a machine where a STAX XML file resides to the cache.
     * 
     * @param machine the machine where a STAX xml file resides
     * @param fileSep the file separater for the machine where a
     * STAX xml file resides
     */
    public void addMachine(String machine, String fileSep)
    {
        machine = STAXFileCache.formatEndpoint(machine);
        
        MachineCacheEntry item = new MachineCacheEntry(machine, fileSep);
        
        // Don't add the document if the cache is off

        if (maxCacheSize == 0 )
        {
            return;
        }
        
        // If the cache size isn't unlimited (-1), if the cache is full,
        // perform clean up

        if ((maxCacheSize != -1) && (cache.size() >= maxCacheSize))
        {
            synchronized(cache)
            {
                while(cache.size() >= maxCacheSize)
                {
                    removeOldest();
                }
            }
        }

        cache.put(item.getKey(), item);
    }
    
    /**
     * Removes extra items from the cache when the STAX_CACHE_SIZE setting
     * changes.
     */
    public void cleanup()
    {
        // If the cache size isn't unlimited (-1), if the cache is full,
        // perform clean up

        if ((maxCacheSize != -1) && (cache.size() > maxCacheSize))
        {
            synchronized(cache)
            {
                while(cache.size() > maxCacheSize)
                {
                    removeOldest();
                }
            }
        }
    }
    
    /**
     * Removes the oldest entry in the cache.
     * 
     * Must call when synchronized on the cache object.
     */
    private void removeOldest()
    {
        // Find the oldest item and remove it
        
        Iterator<MachineCacheEntry> iter = cache.values().iterator();

        if (!iter.hasNext())
        {
            // No items in the cache
            return;
        }

        MachineCacheEntry oldest = iter.next();
        
        while(iter.hasNext())
        {
            MachineCacheEntry cached = iter.next();

            if (cached.getLastHitDate().before(oldest.getLastHitDate()))
            {
                oldest = cached;
            }
        }
        
        cache.remove(oldest.getKey());
    }
    
    /**
     * Sets the maximum number of STAX files to cache.
     * 
     * @param size the maximum STAX files that will be stored in cache.
     */
    public void setMaxCacheSize(int size)
    {
        this.maxCacheSize = size;
        cleanup();
    }
    
    /**
     * Gets the maximum number of STAX files that can be held in cache.
     * 
     * @return the maximum cache size.
     */
    public int getMaxCacheSize()
    {
        return this.maxCacheSize;
    }
    
    private static class CacheKey
    {
        private String machine;
        
        public CacheKey(String machine)
        {
            this.machine = machine.toLowerCase();
        }

        public String getMachine()
        {
            return machine;
        }
        
        /* (non-Javadoc)
         * @see java.lang.Object#equals(java.lang.Object)
         */
        public boolean equals(Object o)
        {
            if (this == o)
            {
                return true;
            }
                
            if (o instanceof CacheKey)
            {
                CacheKey k = (CacheKey)o;
                
                return getMachine().equals(k.getMachine());
            }
            
            return false;
        }

        /* (non-Javadoc)
         * @see java.lang.Object#hashCode()
         */
        public int hashCode()
        {
            return machine.hashCode();
        }
        
    }
    
    /**
     * A cached entry for a machine
     */
    public static class MachineCacheEntry
    {
        private String machine;
        private String fileSep;
        private Date date;
        private Date lastHit;
        private long hits;
        
        private CacheKey key;
        
        /**
         * Creates a cached document.
         * 
         * @param doc the document to cache.
         */
        public MachineCacheEntry(String machine, String fileSep)
        {
            this.machine = machine;
            this.fileSep = fileSep;
            this.date = new Date();
            this.lastHit = new Date();
            this.hits = 0;
            
            this.key = new CacheKey(machine);
        }
        
        /**
         * Gets the key used to cache this entry.
         * 
         * @return the cache key for this entry.
         */
        public CacheKey getKey()
        {
            return key;
        }
        
        /**
         * Gets the name of the machine that provided the file.
         * 
         * @return the machine name.
         */
        public String getMachine()
        {
            return machine;
        }
        
        /**
         * Gets the fileSep for the machine.
         * 
         * @return the file separator for the machine.
         */
        public String getFileSep()
        {
            return fileSep;
        }
        
        /**
         * Updates the cache hit for this machine.
         */
        public void hit()
        {
            lastHit = new Date();
            hits++;
        }
        
        /**
         * Gets the number of cache hits for this machine.
         * 
         * @return the number of cache hits.
         */
        public long getHits()
        {
            return hits;
        }
        
        /**
         * Gets last time the machine had a cache hit.
         * 
         * @return the last cache hit time.
         */
        public Date getLastHitDate()
        {
            return lastHit;
        }
        
        /**
         * Gets the date when this machine was added to the cache.
         * 
         * @return the date the machine was added to the cache.
         */
        public Date getAddDate()
        {
            return date;
        }    
        
        /* (non-Javadoc)
         * @see java.lang.Object#equals(java.lang.Object)
         */
        public boolean equals(Object o)
        {
            if (this == o)
            {
                return true;
            }
                
            if (o instanceof MachineCacheEntry)
            {
                MachineCacheEntry o1 = (MachineCacheEntry)o;
                
                // Two machines are equivalent if they are the same machine
                boolean equals = this.getMachine().equalsIgnoreCase(
                    o1.getMachine());
                
                return equals;
            }
            
            return false;
        }
    }
}
