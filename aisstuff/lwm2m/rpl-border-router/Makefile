CONTIKI_PROJECT = border-router
all: $(CONTIKI_PROJECT)
CONTIKI=../../../contiki-ng

# The BR is either native or embedded, and in the latter case must support SLIP
PLATFORMS_EXCLUDE = nrf52dk z1
BOARDS_EXCLUDE = nrf52840/dongle

MAKE_MAC ?= MAKE_MAC_TSCH

# Include RPL BR module
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/rpl-border-router
# Include webserver module
MODULES_REL += webserver

# Include optional target-specific module
include $(CONTIKI)/Makefile.identify-target
MODULES_REL += $(TARGET)

# force Orchestra from command line
MAKE_WITH_ORCHESTRA ?= 1

# RPL storing mode?
MAKE_WITH_STORING_ROUTING ?= 1
# Orchestra link-based rule? (Works only if Orchestra & storing mode routing is enabled)
MAKE_WITH_LINK_BASED_ORCHESTRA ?= 1
# Use the Orchestra root rule?
MAKE_WITH_ORCHESTRA_ROOT_RULE ?= 1

ORCHESTRA_EXTRA_RULES = &unicast_per_neighbor_rpl_ns

ifeq ($(MAKE_WITH_ORCHESTRA),1)
 MODULES += $(CONTIKI_NG_SERVICES_DIR)/orchestra

 ifeq ($(MAKE_WITH_STORING_ROUTING),1)
	 ifeq ($(MAKE_WITH_LINK_BASED_ORCHESTRA),1)
		 # enable the `link_based` rule
		 ORCHESTRA_EXTRA_RULES = &unicast_per_neighbor_link_based
	 else
		 # enable the `rpl_storing` rule
		 ORCHESTRA_EXTRA_RULES = &unicast_per_neighbor_rpl_storing
	 endif

	 ifeq ($(MAKE_WITH_ORCHESTRA_ROOT_RULE),1)
		 # add the root rule
		 ORCHESTRA_EXTRA_RULES +=,&special_for_root
	 endif
 else
	 ifeq ($(MAKE_WITH_LINK_BASED_ORCHESTRA),1)
		 $(error "Inconsistent configuration: link-based Orchestra requires routing info")
	 endif

	 ifeq ($(MAKE_WITH_ORCHESTRA_ROOT_RULE),1)
		$(error "Inconsistent configuration: NS rule and root rule conflicts!")
	 endif
 endif

 # pass the Orchestra rules to the compiler
 CFLAGS += -DORCHESTRA_CONF_RULES="{&eb_per_time_source,$(ORCHESTRA_EXTRA_RULES),&default_common}"
endif

ifeq ($(MAKE_WITH_STORING_ROUTING),1)
 MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC
endif

include $(CONTIKI)/Makefile.include
