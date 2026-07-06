# Copyright (C) 2026 Czy_4201b
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import redstonex as rsx

if __name__ == "__main__":
    sim = rsx.Simulator()
    s = rsx.Source()
    l1 = rsx.Line()
    l2 = rsx.Line()
    s.connect(l1)
    l1.connect(l2)
    
    sim.bind_object(s)
    sim.bind_object(l1)
    sim.bind_object(l2)

    sim.run()

    print(s.power)
    print(l1.power)
    print(l2.power)
