/*
 * Copyright (C) 2021 Adrian Carpenter
 *
 * This file is part of Pingnoo (https://github.com/nedrysoft/pingnoo)
 *
 * An open-source cross-platform traceroute analyser.
 *
 * Created by Adrian Carpenter on 23/05/2021.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PINGNOO_COMPONENTS_ROUTEANALYSER_ROUTEANALYSERMENUITEM_H
#define PINGNOO_COMPONENTS_ROUTEANALYSER_ROUTEANALYSERMENUITEM_H

#include <QWidget>

class QFont;

namespace Nedrysoft { namespace RouteAnalyser {
    /**
     * @brief       The RouteAnalyserMenuItem draws an item in the operating system menu icon, a list of items
     *              are shown for currently active analysis targets, the item is a mini overview of a target.
     */
    class RouteAnalyserMenuItem :
            public QWidget {

        private:
            Q_OBJECT

        public:
            /**
             * @brief       Creates a new menu item which is a child of the parent.
             *
             * @param[in]   parent the parent of this child.
             */
            RouteAnalyserMenuItem(QWidget *parent=nullptr);

            /**
             * @brief       Destroys the RouteAnaluserMenuItem.
             */
            ~RouteAnalyserMenuItem();

        private:
            int m_robotoMono;
    };
}}


#endif //PINGNOO_COMPONENTS_ROUTEANALYSER_ROUTEANALYSERMENUITEM_H
