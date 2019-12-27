/*****************************************************************************/
/* Software Testing Automation Framework (STAF)                              */
/* (C) Copyright IBM Corp. 2002, 2007                                        */
/*                                                                           */
/* This software is licensed under the Eclipse Public License (EPL) V1.0.    */
/*****************************************************************************/

package com.ibm.staf.service.stax;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Comparator;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.ibm.staf.STAFResult;
import com.ibm.staf.STAFUtil;
import com.ibm.staf.STAFHandle;

/**
 * Caches STAX XML files.
 */
public class STAXFileCache
{
    private static final int DEFAULT_CACHE_SIZE = 20;

    /**
     *  LRU is the "Least Recently Used" cache replacement algorithm.
     *  If a new entry needs to be added to the cache and the cache is full,
     *  the least recently used cache entry will be removed.
     */
    public static final int LRU = 1;
    private static final String LRU_STRING = "LRU";

    /**
     *  LFU is the "Least Frequently Used" cache replacement algorithm
     *  that can also be configured to get rid of the oldest "stale" entry.
     *  If a new entry needs to be added to the cache and the cache is full,
     *  - If maxAge = 0 (the default indicating no entries are considered to
     *    be stale), the least frequently used cache entry will be removed
     *  - if maxAge > 0, if any cache entry is stale, the mleast recently used
     *    stale entry will be removed.  Otherwise, if no cache entries are
     *    stale, the least frequently used cache entry will be removed.
     */ 
    public static final int LFU = 2;
    private static final String LFU_STRING = "LFU";
    
    private static STAXFileCache instance;

    /**
     * A map contain the cache entries
     */ 
    private Map<CacheKey, FileCacheEntry> cache;

    /**
     * Maximum number of entries that will be stroed in the cache
     */
    private int maxCacheSize;

    /**
     * Caching algorithm (LRU=1 or LFU=2)
     */ 
    private int algorithm;

    /**
     *  Maximum age before a cached document is considered "stale"
     * if using the LFU algorithm.  0 indicates no maximum age.
     */ 
    private MaxAge maxAge;

    /**
     *  The number of times a cache hit has occurred for any item in the cache
     */ 
    private long cacheHitCount = 0;

    /**
     * The number of times a cache miss has occurred for any item not in cache
     * or that was stale (e.g. not up-to-date)
     */ 
    private long cacheMissCount = 0;

    /**
     * Date that the cache was initialized or last purged
     */ 
    private Date lastPurgeDate;

    private Set<String> localMachineNames;
    
    /**
     * Initializes the STAXCache instance.
     */
    private STAXFileCache()
    {
        maxCacheSize = DEFAULT_CACHE_SIZE;
        algorithm = LRU;  // Default cache algorithm
        maxAge = new MaxAge(); // The default is 0 = No maximum age
        lastPurgeDate = new Date();
        cache = Collections.synchronizedMap(
            new HashMap<CacheKey, FileCacheEntry>());

        localMachineNames = new HashSet<String>();
        addLocalMachineNames();
    }
    
    /**
     * Adds known local machine name aliases
     */
    private void addLocalMachineNames()
    {
        localMachineNames.add("local");
        localMachineNames.add("localhost");
        localMachineNames.add("127.0.0.1");
        
        try
        {
            InetAddress addr = InetAddress.getLocalHost();
            
            localMachineNames.add(addr.getHostAddress().toLowerCase());
            localMachineNames.add(addr.getHostName().toLowerCase());
            localMachineNames.add(addr.getCanonicalHostName().toLowerCase());
        }
        catch (UnknownHostException e)
        {
            // Do nothing
        }
    }
    
    /**
     * Adds a known local machine name alias.
     * 
     * @param name a local machine name alias such as "localhost".
     */
    public void addLocalMachineName(String name)
    {
        localMachineNames.add(name.toLowerCase());
    }
    
    /**
     * Gets the cache instance.
     * 
     * @return the STAX document cache.
     */
    public static STAXFileCache get()
    {
        if (instance == null)
        {
            instance = new STAXFileCache();
        }
        
        return instance;
    }
    
    /**
     * Get the file separator for the operating system of the specified
     * machine.
     * 
     * @param machine A string containing the endpoint of a machine
     * @param handle  A STAFHandle object to use to submit the VAR RESOLVE
     *                request
     * @return STAFResult If successful, rc = 0 and result contains the file 
     * separator.  If fails, rc = non-zero and result contains an error
     * message.
     */
    public static STAFResult getFileSep(String machine, STAFHandle handle)
    {
        String fileSep = "";

        if (STAXFileCache.get().isLocalMachine(machine))
        {
            // Assign the file separator for the local STAX machine
            fileSep = STAX.fileSep;
            return new STAFResult(STAFResult.Ok, fileSep);
        }
        
        // Get the file separator for the remote machine.
          
        // Get the file separator from the machine cache if the remote
        // machine is in the cache (so that don't have to submit a VAR
        // RESOLVE request to the remote machine).
         
        if (STAXMachineCache.get().checkCache(machine))
        {
            fileSep = STAXMachineCache.get().getFileSep(machine);
            return new STAFResult(STAFResult.Ok, fileSep);
        }
        
        // The remote machine is not in the machine cache, so submit a
        // VAR RESOLVE request to get the file separator and add the
        // machine/fileSep to the machine cache.

        STAFResult result = handle.submit2(
            machine, "VAR", "RESOLVE STRING {STAF/Config/Sep/File}");

        if (result.rc == STAFResult.Ok)
        {
            fileSep = result.result;

            // Add the machine/fileSep to the machine cache
            STAXMachineCache.get().addMachine(machine, fileSep);
        }
        
        return result;
    }


    /**
     * Checks if the given machine name is a known alias for
     * the local machine.
     * 
     * @param machine the machine name to check.
     * @return whether the machine is known to be an alias to the 
     * local machine.
     */
    public boolean isLocalMachine(String machine)
    {
        return localMachineNames.contains(formatEndpoint(machine));
    }
    
    /**
     * Formats a machine endpoint. This takes a STAF endpoint in the
     * format [&lt;Interface>://]<Logical or Physical Identifier>[@&lt;Port>]
     * and strips off the interface and port leaving only the logical
     * or physical identifier. The identifier will also be converterd to
     * lowercase.
     * 
     * @param machine the machine (STAF endpoint) name to format.
     * @return the logical or physical identifier of the endpoint.
     */
    public static String formatEndpoint(String endpoint)
    {
        // Strip the port
        endpoint = STAFUtil.stripPortFromEndpoint(endpoint);

        // Strip the interface
        int index = endpoint.indexOf("://");
        if (index > -1)
        {
            endpoint = endpoint.substring(index + 3);
        }

        endpoint = endpoint.toLowerCase();
        
        return endpoint;
    }
    
    /**
     * Gets a STAX document from the cache. This does not check to see if the
     * cached copy is up-to-date. The method checkCache should first be called
     * (if necessary) to determine if the cached copy exists and is up-to-date.
     * 
     * @param machine the machine containg the XML file.
     * @param filename the STAX XML file name.
     * @param caseSensitiveFileName a flag indicating if the filename is
     *                              case-sensitive
     * @return a cached document or null if the document is not in the cache.
     */
    public STAXDocument getDocument(String machine, String filename,
                                    boolean caseSensitiveFileName)
    {
        STAXDocument doc = null;
        
        machine = formatEndpoint(machine);
        
        if (isLocalMachine(machine))
        {
            machine = "local";
        }
        
        FileCacheEntry item = cache.get(
            new CacheKey(machine, filename, caseSensitiveFileName));
        
        if (item != null)
        {
            doc = item.getDocument().cloneDocument();
            item.hit();
            cacheHitCount++;
        }
        
        return doc;
    }
    
    /**
     * Gets the contents of the cache in the sort order specified.
     * 
     * @return a sorted list of STAXCache.CachedSTAX items.
     */
    public List<FileCacheEntry> getCacheContents(int sortBy)
    {
        List<FileCacheEntry> cacheContents = null;
        
        synchronized(cache)
        {
            cacheContents = new LinkedList<FileCacheEntry>(cache.values());
        }
        
        if (sortBy == STAXFileCache.LRU)
        {
            // Sort the contents by the last hit date in descending order

            Collections.sort(
                cacheContents,
                new Comparator<STAXFileCache.FileCacheEntry>()
            {
                public int compare(STAXFileCache.FileCacheEntry entry1,
                                   STAXFileCache.FileCacheEntry entry2)
                {
                    // Sort by most recent hit date first
                    return entry1.getLastHitDate().compareTo(
                        entry2.getLastHitDate()) * -1;
                }
            });
        }
        else if (sortBy == STAXFileCache.LFU)
        {
            // Sort the contents as follows:
            // - Sort first by whether stale or not.  Put the non-stale
            //   documents before all stale documents.  Sort the non-stale
            //   documents by the greatest number of hits, and if the same
            //   number of hits, then by recent hit date first.
            // - Sort the stale documents by the most recent hit date first.

            Collections.sort(
                cacheContents,
                new Comparator<STAXFileCache.FileCacheEntry>()
            {
                public int compare(STAXFileCache.FileCacheEntry e1,
                                   STAXFileCache.FileCacheEntry e2)
                {
                    boolean e1_isStale = isStale(e1.getLastHitDate());
                    boolean e2_isStale = isStale(e2.getLastHitDate());
                    
                    if (e1_isStale == false && e2_isStale == false)
                    {
                        // Both entries are not stale.
                        // Sort by greatest number of hits first

                        int compareResult = (new Long(e1.getHits())).
                            compareTo(new Long(e2.getHits()));

                        if (compareResult == 0)
                        {
                            // Same number of hits

                            return e1.getLastHitDate().compareTo(
                                e2.getLastHitDate()) * -1;
                        }
                        else
                        {
                            return compareResult * -1;
                        }
                    }
                    else if (e1_isStale == true && e2_isStale == true)
                    {
                        // Both entries are stale.
                        // Sort by recent hit date first.

                        return e1.getLastHitDate().compareTo(
                            e2.getLastHitDate()) * -1;
                    }
                    else if (e1_isStale == false)
                    {
                        // e1_isStale == false && e2_isStale == true
                        return -1;
                    }
                    else
                    {
                        // e1_isStale == true && e2_isStale == false
                        return 1;
                    }
                }
            });
        }
        
        return cacheContents;
    }
    
    /**
     * Clears the contents of the cache and clears the cache statistics.
     * 
     * @return the number of files cleared from the cache.
     */
    public int purge()
    {
        synchronized(cache)
        {
            int cleared = cache.size();
            cache.clear();
            lastPurgeDate = new Date();

            // Clear the cache statistics
            cacheHitCount = 0;
            cacheMissCount = 0;
            
            return cleared;
        }
    }
    
    /**
     * Converts a STAX date string to a java Date object.
     * 
     * @param staxDate the STAX date string.
     * @return a java Date object.
     */
    public static Date convertSTAXDate(String staxDate)
    {
        String sYear = staxDate.substring(0,4);
        String sMonth = staxDate.substring(4,4+2);
        String sDay = staxDate.substring(6,6+2);
        String sHour = staxDate.substring(9,9+2);
        String sMinute = staxDate.substring(12,12+2);
        String sSecond = staxDate.substring(15,15+2);
        
        int year = Integer.parseInt(sYear);
        
        // Calendar expects month to be zero-based (0 = January)
        int month = Integer.parseInt(sMonth) - 1;
        int day = Integer.parseInt(sDay);
        int hour = Integer.parseInt(sHour);
        int minute = Integer.parseInt(sMinute);
        int second = Integer.parseInt(sSecond);
        
        Calendar cal = Calendar.getInstance();
        cal.set(year, month, day, hour, minute, second);
        
        // Chop off the milliseconds portion
        // (Should be zero but Calendar sets it to the current system
        // milliseconds)
        long time = ((long)(cal.getTimeInMillis() / 1000))*1000;
        
        return new Date(time);
    }
    
    /**
     * Checks the cache to see if it contains an up-to-date copy of a
     * STAX document.
     * 
     * @param machine the machine that contains the STAX XML.
     * @param filename the filename of the STAX XML.
     * @param date the current last modification date of the file.
     * @param caseSensitiveFileName a flag indicating if the filename is
     *                              case-sensitive
     * @return true if the item is cached, false if the item is not cached or
     *             the cached copy is old.
     */
    public boolean checkCache(String machine, String filename, Date date,
                              boolean caseSensitiveFileName)
    {
        machine = formatEndpoint(machine);
        
        if (isLocalMachine(machine))
        {
            machine = "local";
        }
        
        FileCacheEntry item = cache.get(
            new CacheKey(machine, filename, caseSensitiveFileName));
        
        if (item != null)
        {
            // Make sure the modification dates are the same, if not the
            // cache is old.

            if (date.equals(item.getModDate()))
            {
                return true;
            }
        }
        
        return false;
    }
    
    /**
     * Adds a STAX document to the cache, or if it already exists then 
     * that means that the document's modification date has changed, so
     * then it updates the cache entry with the new document, its new
     * modification date, and its last hit date.
     * 
     * @param filename the file name of the STAX xml file.
     * @param doc the parsed STAX document.
     * @param modDate the modification date of the STAX file.
     * @param caseSensitiveFileName a flag indicating if the filename is
     *                              case-sensitive
     */
    public void addDocument(String machine, String filename, STAXDocument doc,
                            Date modDate, boolean caseSensitiveFileName)
    {
        machine = formatEndpoint(machine);
        
        if (isLocalMachine(machine))
        {
            machine = "local";
        }
        
        cacheMissCount++;
        
        // Don't add the document if the cache is off

        if (maxCacheSize == 0 )
        {
            return;
        }

        CacheKey key = new CacheKey(machine, filename, caseSensitiveFileName);
        FileCacheEntry item = null;
        STAXDocument clonedDoc = doc.cloneDocument();

        synchronized(cache)
        {
            item = cache.get(key);

            if (item == null)
            {
                // Add document to the cache

                item = new FileCacheEntry(
                    machine, filename, clonedDoc, modDate,
                    caseSensitiveFileName);

                // If the cache size isn't unlimited (-1) and the cache is
                // full, remove an entry from the cache to make room for
                // this new entry

                if ((maxCacheSize != -1) && (cache.size() >= maxCacheSize))
                {
                    while(cache.size() >= maxCacheSize)
                    {
                        if (algorithm == LRU)
                            removeOldest();
                        else
                            removeLfu();
                    }
                }
            }
            else
            {
                // Already in cache, so update the document, the document's
                // modification date, and the date the document was added/
                // updated in the cache, and the last hit date.

                item.setDocument(clonedDoc);
                item.setModDate(modDate);
                item.setLastHitDate(new Date());
            }

            // Add or update the cache entry

            cache.put(key, item);
        }
    }
    
    /**
     * Removes extra items from the cache when the STAX_CACHE_SIZE setting
     * changes.
     */
    public void cleanup()
    {
        // If the cache size isn't unlimited (-1) and the number of entries
        // in the cache exceeds the maximum cache size (e.g. because the
        // maximum cache size was just decreased), remove entries from the
        // cache so that its total number of entries does not exceed the
        // maximum size.

        if ((maxCacheSize != -1) && (cache.size() > maxCacheSize))
        {
            synchronized(cache)
            {
                while(cache.size() > maxCacheSize)
                {
                    if (algorithm == LRU)
                        removeOldest();
                    else
                        removeLfu();
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
        Iterator<FileCacheEntry> iter = cache.values().iterator();

        if (!iter.hasNext())
        {
            // No items in the cache
            return;
        }

        FileCacheEntry oldest = iter.next();
            
        while(iter.hasNext())
        {
            FileCacheEntry cached = iter.next();

            if (cached.getLastHitDate().before(oldest.getLastHitDate()))
            {
                oldest = cached;
            }
        }

        cache.remove(oldest.getKey());
    }

    /**
     * Removes the entry with the least number of hits, the Least
     * Frequently Used entry in the cache, unless there is a "stale"
     * document in the cache (one that hasn't been accessed within the
     * maxAge time period, assuming maxAge is not 0 which means that
     * no documents will be considered to be stale).  Then, the most
     * "stale" document in the cache will be removed instead.
     * 
     * Must call when synchronized on the cache object.
     */
    private void removeLfu()
    {
        // Find the entry with the least number of hits and remove it

        Iterator<FileCacheEntry> iter = cache.values().iterator();

        if (!iter.hasNext())
        {
            // No items in the cache
            return;
        }
                
        FileCacheEntry lowest = iter.next();
        FileCacheEntry oldest = lowest;
            
        while(iter.hasNext())
        {
            FileCacheEntry cached = iter.next();

            if (maxAge.getAmount() != 0)
            {
                // A maximum age has been specified so determine the
                // oldest entry in the cache

                if (cached.getLastHitDate().before(oldest.getLastHitDate()))
                {
                    oldest = cached;
                }
            }

            // Determine the entry with the lowest number of hits and
            // the least recent hit date

            if (cached.getHits() < lowest.getHits())
            {
                lowest = cached;
            }
            else if (cached.getHits() == lowest.getHits())
            {
                // Set to the entry that is oldest

                if (cached.getLastHitDate().before(lowest.getLastHitDate()))
                {
                    lowest = cached;
                }
            }
        }

        // If a maximum age has been configured (e.g. != 0), check if the
        // document with the oldest last hit date is stale and if so,
        // remove it and return

        if (isStale(oldest.getLastHitDate()))
        {
            cache.remove(oldest.getKey());

            return;
        }

        // Otherwise, remove the document with the least number of hits
        // (the Least Frequently Used entry)

        cache.remove(lowest.getKey());
    }
    
    /**
     * Check if a document is stale by checking if a document's last hit
     * timestamp plus the maxAge is earlier than the current timestamp.
     */ 
    private boolean isStale(Date lastHitDate)
    {
        // A maxAge amount of 0 means no maximum age, so no document will
        // be considered to be stale

        if (maxAge.getAmount() == 0)
            return false;

        Calendar calendar = new GregorianCalendar();

        // Tell the calendar to the last hit date/time passed in
        calendar.setTime(lastHitDate);

        // Add the time it takes before a document is considered stale to the
        // last hit date

        calendar.add(maxAge.getTimeField(), maxAge.getAmount());

        // Check if the last hit timestamp + maxAge is earlier than the
        // current date/time

        return calendar.before(Calendar.getInstance());
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

    /**
     * Sets the cache algorithm used to determine the entry to be
     * removed when the cache is full.
     * 
     * @return A STAFResult object with rc 0 if the algorithm was
     *         set successfully.  If an invalid algorithm is
     *         specified, its rc will be set to a non-zero value
     *         with an error message in its result.
     */
    public STAFResult setAlgorithm(String algorithm)
    {
        if (algorithm.equalsIgnoreCase(LRU_STRING))
        {
            this.algorithm = LRU;
        }
        else if (algorithm.equalsIgnoreCase(LFU_STRING))
        {
            this.algorithm = LFU;
        }
        else
        {
            return new STAFResult(
                STAFResult.InvalidValue,
                "FILECACHEALGORITHM must be set to LRU or LFU.  " +
                "Invalid value: " + algorithm);
        }

        return new STAFResult(STAFResult.Ok);
    }

    /**
     * Gets the cache algorithm used to determine the entry to be
     * removed when the cache is full.
     * 
     * @return the cache algorithm.
     */
    public int getAlgorithm()
    {
        return this.algorithm;
    }

    /**
     * Gets the cache algorithm in its String form
     * 
     * @return the cache algorithm's name.
     */
    public String getAlgorithmString()
    {
        if (this.algorithm == LRU)
            return LRU_STRING;
        else
            return LFU_STRING;
    }
    
    /**
     * Sets the maximum age for cached STAX documents.
     * This is used only when the cache algorithm is LFU to determine
     * if a document is stale.
     * 
     * @param age the maximum age for a cached document (e.g. "30",
     * "30s", "5m", "2h", "1d", "1w").
     */
    public STAFResult setMaxAge(String maxAge)
    {
        try
        {
            this.maxAge = new MaxAge(maxAge);
        }
        catch (Exception e)
        {
            return new STAFResult(STAFResult.InvalidValue, e.getMessage());
        }

        return new STAFResult(STAFResult.Ok);
    }
    
    /**
     * Gets the maximum age for cached STAX documents.
     * 
     * @return the maximum age.
     */
    public MaxAge getMaxAge()
    {
        return this.maxAge;
    }

    public long getCacheHitCount()
    {
        return this.cacheHitCount;
    }

    public long getCacheMissCount()
    {
        return this.cacheMissCount;
    }

    public Date getLastPurgeDate()
    {
        return this.lastPurgeDate;
    }
    
    /**
     * This class's constructor sets its time field and amount field
     * based on the maxAge string that is passed in the format of
     * <Number>[s|m|h|d|w] so that it can be used to "add" the time
     * to the last hit date to determine if a document is stale.
     */ 
    private static class MaxAge
    {
        private static String sMAX_AGE_ERROR_MSG =
            "The MAXFILECACHEAGE value may be expressed in seconds, " +
            "minutes, hours, days, or weeks.  Its format is " +
            "<Number>[s|m|h|d|w] where <Number> must be an integer from 0 " +
            "to 2147483647 and indicates seconds unless one of the " +
            "following case-insensitive suffixes is specified:  " +
            "s (for seconds), m (for minutes), h (for hours), " +
            "d (for days), or w (for weeks).  If &lt;Number> is 0, this " +
            "means that there is no maximum age.\n\nExamples: \n" +
            "  0 specifies no maximum age, \n" +
            "  30 specifies 30 seconds, \n" +
            "  30s specifies 30 seconds, \n" +
            "  5m specifies 5 minutes, \n" +
            "  2h specifies 2 hours, \n" +
            "  1d specifies 1 day, \n" +
            "  1w specifies 1 week.";

        /**
         * The value represented as a string (e.g. "30", "30s", "1m", "2h",
         * "1d", "1w")
         */
        private String displayString = "0";

        /**
         * The time field (e.g. Calendar.SECOND, Calendar.MINUTE,
         * Calendar.HOUR_OF_DAY, Calendar.Date, or Calendar.WEEK_OF_YEAR
         */
        private int timeField = Calendar.SECOND;

        /**
         *  The amount of date or time to be added
         */ 
        private int amount = 0;

        private MaxAge()
        {
            // Do nothing.  Sets to 0 which means No maximum age
        }

        public MaxAge(String maxAge) throws Exception
        {
            maxAge = maxAge.trim();

            if ((maxAge == null) || (maxAge.length() == 0))
                throw new Exception(sMAX_AGE_ERROR_MSG);

            displayString = maxAge;

            // Check if the max age string is not all digits

            try
            {
                amount = (new Integer(maxAge)).intValue();
            }
            catch (NumberFormatException e)
            {
                // Get max age type (last character of max age string)

                String maxAgeType = maxAge.substring(
                    maxAge.length() - 1).toLowerCase();

                if (maxAgeType.equals("s"))
                    timeField = Calendar.SECOND;
                else if (maxAgeType.equals("m"))
                    timeField = Calendar.MINUTE;
                else if (maxAgeType.equals("h"))
                    timeField = Calendar.HOUR_OF_DAY;
                else if (maxAgeType.equals("d"))
                    timeField = Calendar.DATE;
                else if (maxAgeType.equals("w"))
                    timeField = Calendar.WEEK_OF_YEAR;
                else
                    throw new Exception(sMAX_AGE_ERROR_MSG);

                // Assign numeric max age (all characters except the last one)

                try
                {
                    amount = (new Integer(
                        maxAge.substring(0, maxAge.length() - 1))).intValue();
                }
                catch (NumberFormatException e2)
                {
                    throw new Exception(sMAX_AGE_ERROR_MSG);
                }
            }

            if (amount < 0)
                throw new Exception(sMAX_AGE_ERROR_MSG);
        }

        public String toString()
        {
            return displayString;
        }

        public int getTimeField()
        {
            return timeField;
        }
        
        public int getAmount()
        {
            return amount;
        }
    }

    private static class CacheKey
    {
        private String machine;
        private String filename;
        
        public CacheKey(String machine, String filename,
                        boolean caseSensitiveFileName)
        {
            this.machine = machine.toLowerCase();

            if (caseSensitiveFileName)
                this.filename = filename;
            else
                this.filename = filename.toLowerCase();
        }

        public String getMachine()
        {
            return machine;
        }
        
        public String getFilename()
        {
            return filename;
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
                
                return getMachine().equals(k.getMachine()) &&
                    getFilename().equals(k.getFilename());
            }
            
            return false;
        }

        /* (non-Javadoc)
         * @see java.lang.Object#hashCode()
         */
        public int hashCode()
        {
            return machine.hashCode() + filename.hashCode();
        }
        
    }
    
    /**
     * A Cached STAX document.
     */
    public static class FileCacheEntry
    {
        private String machine;
        private String filename;
        private boolean caseSensitiveFileName = true;
        
        private Date date;
        private Date modDate;
        private Date lastHit;
        private long hits;
        private STAXDocument doc;
        
        private CacheKey key;
        
        /**
         * Creates a cached document.
         * 
         * @param doc the document to cache.
         */
        public FileCacheEntry(String machine, String filename,
                              STAXDocument doc, Date modDate,
                              boolean caseSensitiveFileName)
        {
            this.machine = machine;
            this.filename = filename;
            this.caseSensitiveFileName = caseSensitiveFileName;
            this.doc = doc;
            this.modDate = modDate;
            this.date = new Date();
            this.lastHit = new Date();
            this.hits = 0;
            this.key = new CacheKey(machine, filename, caseSensitiveFileName);
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
         * Gets the name/path to the file as it exists on the providing
         * machine.
         * 
         * @return the name/path to the file.
         */
        public String getFilename()
        {
            return filename;
        }
        
        /**
         * Gets a flag indicating if file names are case-sensitive
         * 
         * @return true if the file name is case-sensitive; false if not
         * case-sensitive.
         */
        public boolean getCaseSensitive()
        {
            return caseSensitiveFileName;
        }

        /**
         * Gets the STAX document.
         * 
         * @return the cached STAX document.
         */
        public STAXDocument getDocument()
        {
            return doc;
        }

        /**
         * Sets the STAX document.
         */ 
        public void setDocument(STAXDocument doc)
        {
            this.doc = doc;
        }
        
        /**
         * Updates the cache hit for this document.
         */
        public void hit()
        {
            lastHit = new Date();
            hits++;
        }
        
        /**
         * Gets the number of cache hits for this document.
         * 
         * @return the number of cache hits.
         */
        public long getHits()
        {
            return hits;
        }
        
        /**
         * Gets the date when the STAX file was last modified.
         * 
         * @return
         */
        public Date getModDate()
        {
            return modDate;
        }
        
        /**
         * Sets the date when the STAX file was last modified.
         */ 
        public void setModDate(Date modDate)
        {
            this.modDate = modDate;
        }
        
        /**
         * Gets last time the document in the cache was accessed.
         * 
         * @return the last cache hit time.
         */
        public Date getLastHitDate()
        {
            return lastHit;
        }

        /**
         * Sets the date when the document was last accessed in the
         * cache.
         */ 
        public void setLastHitDate(Date lastHit)
        {
            this.lastHit = lastHit;
        }
        
        /**
         * Gets the date when this document was added to the cache.
         * 
         * @return the date the document was added to the cache.
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
                
            if (o instanceof FileCacheEntry)
            {
                FileCacheEntry o1 = (FileCacheEntry)o;
                
                // Two documents are equivalent if they have the same machine, 
                // the same filename (case-insensitive if Windows and case-
                // sensitive if Unix), and the same last modified date

                boolean equals = 
                    this.getMachine().equalsIgnoreCase(o1.getMachine()) &&
                    ((this.getCaseSensitive() &&
                      this.getFilename().equals(o1.getFilename())) ||
                     (!this.getCaseSensitive() &&
                      this.getFilename().equalsIgnoreCase(o1.getFilename()))) &&
                    this.getModDate().equals(o1.getModDate());
                
                return equals;
            }
            
            return false;
        }
    }
}
