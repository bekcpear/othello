from othello import *

_ = gettext

class SilverFish(Player):
    def do_get_turn(self, board):
        asize = 0
        ax = 0
        ay = 0
        for x in range(8):
            for y in range(8):
                alist = []
                result = board.try_go(x, y, self.get_myside(), 1, alist)
                if result == GO_RESULT.GO_OK :
                    if len(alist) >  asize :
                        asize = len(alist)
                        ax = x
                        ay = y
                        pass
                    pass
                pass
            pass
        self.go(ax, ay)

class SilverFishCreator(PlayerCreator):
    def create(self, args):
        return SilverFish()

creators.register_player('Silverfish',
                         _('Silverfish Player'),
                         _('Silverfish player. '
                           'Go position where the most amount of grids would be turned'),
                         SilverFishCreator())

