# countTiles*のコンパイルとテスト(8分程度掛かる)
# 実行方法
# $ make (引数なし)

TARGET_CPP=countTilesCpp
TARGET_HS=countTilesHs
TARGET_HS_SLOW=countTilesSlow
TARGET_HS_SHORT=countTilesShort
TARGET_HS_EX=countTilesEx
TARGETS=$(TARGET_CPP) $(TARGET_HS) $(TARGET_HS_SLOW) $(TARGET_HS_SHORT) $(TARGET_HS_EX)

SOURCE_CPP=countTiles.cpp
SOURCE_HS=countTiles.hs
SOURCE_HS_SLOW=countTilesSlow.hs
SOURCE_HS_SHORT=countTilesShort.hs
SOURCE_HS_EX=countTilesEx.hs
SOURCE_RUBY=countTiles.rb
HS_CHECK=countTilesCheckHs.rb

LOG_CPP=logCpp.txt
LOG_HS=logHs.txt
LOG_RUBY=logRuby.txt
LOG_HS_SLOW=logHsSlow.txt
LOG_HS_SHORT=logHsShort.txt
LOG_HS_EX=logHsEx.txt
LOGS=$(LOG_CPP) $(LOG_HS) $(LOG_RUBY) $(LOG_HS_SLOW) $(LOG_HS_SHORT) $(LOG_HS_EX)

# 出力形式が変わったら変える
NUMBER_OR_PATTERNS=93600
NUMBER_OR_NONE_LINES=53530
# LF改行のbyteサイズ
SIZE_OF_LOG=4213870

countcases=test -f $1 && grep : $1 | wc | grep " $(NUMBER_OR_PATTERNS) "
execute=time $1 $2 | grep -v pass | tr -d \\r > $3
countnoneline=`test -f $1 && grep none $1 | wc -l`
getfilesize=`test -f $1 && ls -nl $1 | cut --field=5 -d " "`

CXX=clang++
HASKELL=ghc
RUBY=ruby

# virtualをなくすと速くなる
EXTRA_CPPFLAGS=-DDISABLE_VIRTUAL
CPPFLAGS=-std=c++14 -Wall -O2 $(EXTRA_CPPFLAGS)
LIBS=
LDFLAGS=
HASKELLFLAGS=-O

.PHONY: all check checklong clean rebuild

all: check checklong

check: $(TARGETS)
	$(call execute, ./$(TARGET_CPP), , $(LOG_CPP))
	$(call countcases, $(LOG_CPP))
	grep invalid $(LOG_CPP) | wc | grep " 0 "
	test $(call countnoneline, $(LOG_CPP)) -eq $(NUMBER_OR_NONE_LINES)
	test $(call getfilesize, $(LOG_CPP)) -eq $(SIZE_OF_LOG)
	$(call execute, ./$(TARGET_HS), test, $(LOG_HS))
	$(call countcases, $(LOG_HS))
	$(call execute,$(RUBY) $(SOURCE_RUBY), 1, $(LOG_RUBY))
	$(call countcases, $(LOG_RUBY))
	test $(call countnoneline, $(LOG_CPP)) -eq $(call countnoneline, $(LOG_HS))
	test $(call countnoneline, $(LOG_CPP)) -eq $(call countnoneline, $(LOG_RUBY))
	test $(call getfilesize, $(LOG_CPP)) -eq $(call getfilesize, $(LOG_HS))
	test $(call getfilesize, $(LOG_CPP)) -eq $(call getfilesize, $(LOG_RUBY))
	$(RUBY) countTilesCompareLog.rb

# 数分かかる
checklong: $(TARGET_HS_SLOW) $(TARGET_HS_SHORT) $(TARGET_HS_EX)
	$(RUBY) $(HS_CHECK)

$(TARGET_CPP): $(SOURCE_CPP)
	$(CXX) $(CPPFLAGS) -o $@ $< $(LIBS) $(LDFLAGS)

$(TARGET_HS): $(SOURCE_HS)
	$(HASKELL) $(HASKELLFLAGS) -o $@ $<

$(TARGET_HS_SLOW): $(SOURCE_HS_SLOW)
	$(HASKELL) $(HASKELLFLAGS) -o $@ $<

$(TARGET_HS_SHORT): $(SOURCE_HS_SHORT)
	$(HASKELL) $(HASKELLFLAGS) -o $@ $<

$(TARGET_HS_EX): $(SOURCE_HS_EX)
	$(HASKELL) $(HASKELLFLAGS) -XBangPatterns -o $@ $<

clean:
	$(RM) $(TARGETS) $(LOGS) ./*.o ./*.hi

rebuild: clean all
