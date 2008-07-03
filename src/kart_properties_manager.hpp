//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2006 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_KARTPROPERTIESMANAGER_H
#define HEADER_KARTPROPERTIESMANAGER_H

#include <vector>
//#include "kart_properties.hpp"
class KartProperties;

class KartPropertiesManager
{
private:
    std::vector<std::string> m_all_groups;

protected:
    float m_max_steer_angle;

    typedef std::vector<KartProperties*> KartPropertiesVector;
    /** All available kart configurations */
    KartPropertiesVector m_karts_properties;

public:
    KartPropertiesManager();
    ~KartPropertiesManager();
	
	// vector containing kart numbers that have been selected in multiplayer
	// games.  This it used to ensure the same kart can not be selected more
	// than once.
    std::vector<int>		 m_selected_karts;  
    
    const KartProperties*    getKartById            (int i) const;
    const KartProperties*    getKart                (const std::string IDENT) const;
    const int                getKartId              (const std::string IDENT) const;
    int                      getKartByGroup         (const std::string& group, int i) const;
    void                     loadKartData           (bool dont_load_models=false);
    const float              getMaximumSteeringAngle() const {return m_max_steer_angle;}
    const unsigned int       getNumberOfKarts       () const {return (unsigned int)m_karts_properties.size();}
    const std::vector<std::string>& getAllGroups    () const {return m_all_groups;     }
    /** Return len random karts */
    std::vector<std::string> getRandomKarts         (int len);

    /** Fill the empty positions in the given vector with random karts */
    void fillWithRandomKarts (std::vector<std::string>& vec);
    void removeTextures      ();
};

extern KartPropertiesManager *kart_properties_manager;

#endif

/* EOF */
