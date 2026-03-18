/* 
 *	Copyright (C) 2008 Stephen Danauskas
 *	
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */


namespace StochasticModeling
{
    /// <remarks>
    /// The GraphCollection class maintains a collection of all of the graphs that have been used in the program.
    /// For the purely model independent fits, the main two graphs will consist of the updated front panel graphs,
    /// and for the MD fits, the last fit's graph will be saved
    /// </remarks>
    public sealed class GraphCollection
    {
        /// <summary>
        /// The instance of the GraphCollection used to access publicly available members
        /// </summary>
        public static readonly GraphCollection Instance = new GraphCollection();

        private static Graphing m_gMainReflectivityFit;
        private static Graphing m_gMainRhoFit;
        private static Graphing m_gRhoFit;
        private static Graphing m_gReflFit;
        private static Graphing m_gReflEFit;

        //Lock the update so we don't update while the main graph is also updating

        private bool m_bgraphlock = false;

        // make the default constructor private, so that no can directly create it.
        private GraphCollection()
        {
            m_gRhoFit = new Graphing(string.Empty);
            m_gReflFit = new Graphing(string.Empty);
            m_gMainReflectivityFit = new Graphing(string.Empty);
            m_gMainRhoFit = new Graphing(string.Empty);
            m_gReflEFit = new Graphing(string.Empty);

            MainReflGraph.CreateGraph(null);
            MainRhoGraph.CreateGraph(null);
            RhoGraph.CreateGraph(null);
            ReflGraph.CreateGraph(null);
            ReflEGraph.CreateGraph(null);
        }

        /// <summary>
        /// True locks the MI graphs so that they are not updated while being used by another module, while false releases the lock.
        /// Note that the front panel still updates, however it is not updated in this class
        /// </summary>
        public bool SetGraphUpdateLock
        {
            set
            {
                m_bgraphlock = value;
            }
        }

        /// <summary>
        /// Sets the main reflectivity graph object as long as the lock is not set by SetGraphUpdateLock <see cref="SetGraphUpdateLock"/>
        /// </summary>
        public Graphing MainReflGraph
        {
            get
            {
                return m_gMainReflectivityFit;
            }
            set
            {
                if (m_bgraphlock == false)
                {
                    m_gMainReflectivityFit.Hide = false;
                    m_gMainReflectivityFit.AlterGraph(value);
                }
            }
        }

        /// <summary>
        /// Sets the main electron density graph object as long as the lock is not set by SetGraphUpdateLock <see cref="SetGraphUpdateLock"/>
        /// </summary>
        public Graphing MainRhoGraph
        {
            get
            {
                return m_gMainRhoFit;
            }
            set
            {
                if (m_bgraphlock == false)
                {
                    m_gMainRhoFit.Hide = false;
                    m_gMainRhoFit.AlterGraph(value);
                }
            }
        }

        /// <summary>
        /// Sets the model dependent fit to the model independent electron density profile graph object
        /// </summary>
        public Graphing RhoGraph
        {
            get
            {
                return m_gRhoFit;
            }
            set
            {
                m_gRhoFit.Hide = false;
                m_gRhoFit.AlterGraph(value);
            }
        }

        /// <summary>
        /// Sets the reflectivity graph object for the pure model dependent fit
        /// </summary>
        public Graphing ReflGraph
        {
            get
            {
                return m_gReflFit;
            }
            set
            {
                m_gReflFit.Hide = false;
                m_gReflFit.AlterGraph(value);
            }
        }

        /// <summary>
        /// Sets the electron density profile graph object for the pure model dependent fit
        /// </summary>
        public Graphing ReflEGraph
        {
            get
            {
                return m_gReflEFit;
            }
            set
            {
                m_gReflEFit.Hide = false;
                m_gReflEFit.AlterGraph(value);
            }
        }
    }
}
