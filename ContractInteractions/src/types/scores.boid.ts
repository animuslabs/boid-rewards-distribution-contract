import type {
    Action,
    AssetType,
    BytesType,
    NameType,
    UInt32Type,
    UInt64Type,
    UInt8Type,
} from '@wharfkit/antelope'
import {
    ABI,
    Asset,
    Blob,
    Bytes,
    Name,
    Struct,
    TimePointSec,
    UInt32,
    UInt64,
    UInt8,
} from '@wharfkit/antelope'
import type {ActionOptions, ContractArgs, PartialBy, Table} from '@wharfkit/contract'
import {Contract as BaseContract} from '@wharfkit/contract'
export const abiBlob = Blob.from(
    'DmVvc2lvOjphYmkvMS4yABELY2xlYXJyZWNvcmQAAQpyZWNvcmRfaWRzCHVpbnQ2NFtdCmRpc3RyaWJ1dGUABgdnYW1lX2lkBXVpbnQ4DGN5Y2xlX251bWJlcgZ1aW50MzIJc3RhdF9uYW1lBG5hbWUMdG90YWxfcmV3YXJkBWFzc2V0DnRva2VuX2NvbnRyYWN0BG5hbWUScmV3YXJkX3BlcmNlbnRhZ2VzBWJ5dGVzEGdhbWVfcmVjb3JkX2RhdGEABQdnYW1lX2lkBXVpbnQ4BnBsYXllcgRuYW1lC3N0YXRzX25hbWVzBm5hbWVbXQxzdGF0c192YWx1ZXMIdWludDY0W10PY29tcGxldGlvbl90aW1lDnRpbWVfcG9pbnRfc2VjCmdhbWVjb25maWcABQdnYW1lX2lkBXVpbnQ4DGRpc3BsYXlfbmFtZQZzdHJpbmcIbWV0YWRhdGEGc3RyaW5nBmFjdGl2ZQRib29sC3N0YXRzX25hbWVzBm5hbWVbXQtnYW1lcmVjb3JkcwAJAmlkBnVpbnQ2NAZwbGF5ZXIEbmFtZQdnYW1lX2lkBXVpbnQ4C3N0YXRzX25hbWVzBm5hbWVbXQxzdGF0c192YWx1ZXMIdWludDY0W10MY3ljbGVfbnVtYmVyBnVpbnQzMhNyZXdhcmRzX2Rpc3RyaWJ1dGVkBGJvb2wUZ2FtZV9jb21wbGV0aW9uX3RpbWUOdGltZV9wb2ludF9zZWMMbGFzdF91cGRhdGVkDnRpbWVfcG9pbnRfc2VjDGdsb2JhbGNvbmZpZwAGC2luaXRpYWxpemVkBGJvb2wWY3ljbGVzX2luaXRpYXRpb25fdGltZQ50aW1lX3BvaW50X3NlYxBjeWNsZV9sZW5ndGhfc2VjBnVpbnQzMhRtYXhfY3ljbGVfbGVuZ3RoX3NlYwZ1aW50MzIQbWF4X3Jld2FyZF90aWVycwV1aW50OBVtaW5fcmV3YXJkX3BlcmNlbnRhZ2UFdWludDgMaW5pdGNvbnRyYWN0AAUKc3RhcnRfdGltZQ50aW1lX3BvaW50X3NlYxBjeWNsZV9sZW5ndGhfc2VjBnVpbnQzMhRtYXhfY3ljbGVfbGVuZ3RoX3NlYwZ1aW50MzIQbWF4X3Jld2FyZF90aWVycwV1aW50OBVtaW5fcmV3YXJkX3BlcmNlbnRhZ2UFdWludDgHcGxheWVycwADBnBsYXllcgRuYW1lCG1ldGFkYXRhBnN0cmluZwxnYW1lc19wbGF5ZWQGdWludDMyCnJlY29yZGdhbWUAAQdyZWNvcmRzEmdhbWVfcmVjb3JkX2RhdGFbXQpyZW1vdmVnYW1lAAEHZ2FtZV9pZAV1aW50OAtyZW1vdmV0b2tlbgABDHRva2VuX3N5bWJvbAZzeW1ib2wQcmV3YXJkZGlzdGNvbmZpZwAEB2dhbWVfaWQFdWludDgUZGVzdGluYXRpb25fY29udHJhY3QEbmFtZQ1tZW1vX3RlbXBsYXRlBnN0cmluZxN1c2VfZGlyZWN0X3RyYW5zZmVyBGJvb2wPcmV3YXJkc3JlY29yZGVkAAcCaWQGdWludDY0B2dhbWVfaWQFdWludDgMY3ljbGVfbnVtYmVyBnVpbnQzMglzdGF0X25hbWUEbmFtZQx0b3RhbF9yZXdhcmQFYXNzZXQQcmV3YXJkZWRfcGxheWVycwZuYW1lW10RZGlzdHJpYnV0aW9uX3RpbWUOdGltZV9wb2ludF9zZWMLc2V0ZGlzdGNvbmYABAdnYW1lX2lkBXVpbnQ4FGRlc3RpbmF0aW9uX2NvbnRyYWN0BG5hbWUNbWVtb190ZW1wbGF0ZQZzdHJpbmcTdXNlX2RpcmVjdF90cmFuc2ZlcgRib29sB3NldGdhbWUABAdnYW1lX2lkBXVpbnQ4DGRpc3BsYXlfbmFtZQZzdHJpbmcIbWV0YWRhdGEGc3RyaW5nC3N0YXRzX25hbWVzBm5hbWVbXQhzZXR0b2tlbgADDnRva2VuX2NvbnRyYWN0BG5hbWUMdG9rZW5fc3ltYm9sBnN5bWJvbAdlbmFibGVkBGJvb2wLdG9rZW5jb25maWcAAw50b2tlbl9jb250cmFjdARuYW1lDHRva2VuX3N5bWJvbAZzeW1ib2wHZW5hYmxlZARib29sCQDSpUjda1REC2NsZWFycmVjb3JkAACAyvq4m7FLCmRpc3RyaWJ1dGUAkJG5eVKU3XQMaW5pdGNvbnRyYWN0AACAkoalS5G6CnJlY29yZGdhbWUAAICShqlNpboKcmVtb3ZlZ2FtZQAApoI0q02lugtyZW1vdmV0b2tlbgAA1qQoY5eywgtzZXRkaXN0Y29uZgAAAABAScOywgdzZXRnYW1lAAAAAFNBmrPCCHNldHRva2VuAAcAAHNrUqSkYQNpNjQAAApnYW1lY29uZmlnAHC6FKmrpGEDaTY0AAALZ2FtZXJlY29yZHPA3JoURXNoZANpNjQAAAxnbG9iYWxjb25maWcAAAAAX+VNrANpNjQAAAdwbGF5ZXJzAEDGLqVruLoDaTY0AAAQcmV3YXJkZGlzdGNvbmZpZwAAUhena7i6A2k2NAAAD3Jld2FyZHNyZWNvcmRlZACYW5OiqSDNA2k2NAAAC3Rva2VuY29uZmlnAAAAAAA='
)
export const abi = ABI.from(abiBlob)
export namespace Types {
    @Struct.type('clearrecord')
    export class clearrecord extends Struct {
        @Struct.field(UInt64, {array: true})
        declare record_ids: UInt64[]
    }
    @Struct.type('distribute')
    export class distribute extends Struct {
        @Struct.field(UInt8)
        declare game_id: UInt8
        @Struct.field(UInt32)
        declare cycle_number: UInt32
        @Struct.field(Name)
        declare stat_name: Name
        @Struct.field(Asset)
        declare total_reward: Asset
        @Struct.field(Name)
        declare token_contract: Name
        @Struct.field(Bytes)
        declare reward_percentages: Bytes
    }
    @Struct.type('game_record_data')
    export class game_record_data extends Struct {
        @Struct.field(UInt8)
        declare game_id: UInt8
        @Struct.field(Name)
        declare player: Name
        @Struct.field(Name, {array: true})
        declare stats_names: Name[]
        @Struct.field(UInt64, {array: true})
        declare stats_values: UInt64[]
        @Struct.field(TimePointSec)
        declare completion_time: TimePointSec
    }
    @Struct.type('gameconfig')
    export class gameconfig extends Struct {
        @Struct.field(UInt8)
        declare game_id: UInt8
        @Struct.field('string')
        declare display_name: string
        @Struct.field('string')
        declare metadata: string
        @Struct.field('bool')
        declare active: boolean
        @Struct.field(Name, {array: true})
        declare stats_names: Name[]
    }
    @Struct.type('gamerecords')
    export class gamerecords extends Struct {
        @Struct.field(UInt64)
        declare id: UInt64
        @Struct.field(Name)
        declare player: Name
        @Struct.field(UInt8)
        declare game_id: UInt8
        @Struct.field(Name, {array: true})
        declare stats_names: Name[]
        @Struct.field(UInt64, {array: true})
        declare stats_values: UInt64[]
        @Struct.field(UInt32)
        declare cycle_number: UInt32
        @Struct.field('bool')
        declare rewards_distributed: boolean
        @Struct.field(TimePointSec)
        declare game_completion_time: TimePointSec
        @Struct.field(TimePointSec)
        declare last_updated: TimePointSec
    }
    @Struct.type('globalconfig')
    export class globalconfig extends Struct {
        @Struct.field('bool')
        declare initialized: boolean
        @Struct.field(TimePointSec)
        declare cycles_initiation_time: TimePointSec
        @Struct.field(UInt32)
        declare cycle_length_sec: UInt32
        @Struct.field(UInt32)
        declare max_cycle_length_sec: UInt32
        @Struct.field(UInt8)
        declare max_reward_tiers: UInt8
        @Struct.field(UInt8)
        declare min_reward_percentage: UInt8
    }
    @Struct.type('initcontract')
    export class initcontract extends Struct {
        @Struct.field(TimePointSec)
        declare start_time: TimePointSec
        @Struct.field(UInt32)
        declare cycle_length_sec: UInt32
        @Struct.field(UInt32)
        declare max_cycle_length_sec: UInt32
        @Struct.field(UInt8)
        declare max_reward_tiers: UInt8
        @Struct.field(UInt8)
        declare min_reward_percentage: UInt8
    }
    @Struct.type('players')
    export class players extends Struct {
        @Struct.field(Name)
        declare player: Name
        @Struct.field('string')
        declare metadata: string
        @Struct.field(UInt32)
        declare games_played: UInt32
    }
    @Struct.type('recordgame')
    export class recordgame extends Struct {
        @Struct.field(game_record_data, {array: true})
        declare records: game_record_data[]
    }
    @Struct.type('removegame')
    export class removegame extends Struct {
        @Struct.field(UInt8)
        declare game_id: UInt8
    }
    @Struct.type('removetoken')
    export class removetoken extends Struct {
        @Struct.field(Asset.Symbol)
        declare token_symbol: Asset.Symbol
    }
    @Struct.type('rewarddistconfig')
    export class rewarddistconfig extends Struct {
        @Struct.field(UInt8)
        declare game_id: UInt8
        @Struct.field(Name)
        declare destination_contract: Name
        @Struct.field('string')
        declare memo_template: string
        @Struct.field('bool')
        declare use_direct_transfer: boolean
    }
    @Struct.type('rewardsrecorded')
    export class rewardsrecorded extends Struct {
        @Struct.field(UInt64)
        declare id: UInt64
        @Struct.field(UInt8)
        declare game_id: UInt8
        @Struct.field(UInt32)
        declare cycle_number: UInt32
        @Struct.field(Name)
        declare stat_name: Name
        @Struct.field(Asset)
        declare total_reward: Asset
        @Struct.field(Name, {array: true})
        declare rewarded_players: Name[]
        @Struct.field(TimePointSec)
        declare distribution_time: TimePointSec
    }
    @Struct.type('setdistconf')
    export class setdistconf extends Struct {
        @Struct.field(UInt8)
        declare game_id: UInt8
        @Struct.field(Name)
        declare destination_contract: Name
        @Struct.field('string')
        declare memo_template: string
        @Struct.field('bool')
        declare use_direct_transfer: boolean
    }
    @Struct.type('setgame')
    export class setgame extends Struct {
        @Struct.field(UInt8)
        declare game_id: UInt8
        @Struct.field('string')
        declare display_name: string
        @Struct.field('string')
        declare metadata: string
        @Struct.field(Name, {array: true})
        declare stats_names: Name[]
    }
    @Struct.type('settoken')
    export class settoken extends Struct {
        @Struct.field(Name)
        declare token_contract: Name
        @Struct.field(Asset.Symbol)
        declare token_symbol: Asset.Symbol
        @Struct.field('bool')
        declare enabled: boolean
    }
    @Struct.type('tokenconfig')
    export class tokenconfig extends Struct {
        @Struct.field(Name)
        declare token_contract: Name
        @Struct.field(Asset.Symbol)
        declare token_symbol: Asset.Symbol
        @Struct.field('bool')
        declare enabled: boolean
    }
}
export const TableMap = {
    gameconfig: Types.gameconfig,
    gamerecords: Types.gamerecords,
    globalconfig: Types.globalconfig,
    players: Types.players,
    rewarddist: Types.rewarddistconfig,
    rewardsrec: Types.rewardsrecorded,
    tokenconfig: Types.tokenconfig,
}
export interface TableTypes {
    gameconfig: Types.gameconfig
    gamerecords: Types.gamerecords
    globalconfig: Types.globalconfig
    players: Types.players
    rewarddist: Types.rewarddistconfig
    rewardsrec: Types.rewardsrecorded
    tokenconfig: Types.tokenconfig
}
export type RowType<T> = T extends keyof TableTypes ? TableTypes[T] : any
export type TableNames = keyof TableTypes
export namespace ActionParams {
    export namespace Type {
        export interface game_record_data {
            game_id: UInt8Type
            player: NameType
            stats_names: NameType[]
            stats_values: UInt64Type[]
            completion_time: TimePointSec
        }
    }
    export interface clearrecord {
        record_ids: UInt64Type[]
    }
    export interface distribute {
        game_id: UInt8Type
        cycle_number: UInt32Type
        stat_name: NameType
        total_reward: AssetType
        token_contract: NameType
        reward_percentages: BytesType
    }
    export interface initcontract {
        start_time: TimePointSec
        cycle_length_sec: UInt32Type
        max_cycle_length_sec: UInt32Type
        max_reward_tiers: UInt8Type
        min_reward_percentage: UInt8Type
    }
    export interface recordgame {
        records: Type.game_record_data[]
    }
    export interface removegame {
        game_id: UInt8Type
    }
    export interface removetoken {
        token_symbol: Asset.SymbolType
    }
    export interface setdistconf {
        game_id: UInt8Type
        destination_contract: NameType
        memo_template: string
        use_direct_transfer: boolean
    }
    export interface setgame {
        game_id: UInt8Type
        display_name: string
        metadata: string
        stats_names: NameType[]
    }
    export interface settoken {
        token_contract: NameType
        token_symbol: Asset.SymbolType
        enabled: boolean
    }
}
export interface ActionNameParams {
    clearrecord: ActionParams.clearrecord
    distribute: ActionParams.distribute
    initcontract: ActionParams.initcontract
    recordgame: ActionParams.recordgame
    removegame: ActionParams.removegame
    removetoken: ActionParams.removetoken
    setdistconf: ActionParams.setdistconf
    setgame: ActionParams.setgame
    settoken: ActionParams.settoken
}
export type ActionNames = keyof ActionNameParams
export class Contract extends BaseContract {
    constructor(args: PartialBy<ContractArgs, 'abi' | 'account'>) {
        super({
            client: args.client,
            abi: abi,
            account: args.account || Name.from('scores.boid'),
        })
    }
    action<T extends ActionNames>(
        name: T,
        data: ActionNameParams[T],
        options?: ActionOptions
    ): Action {
        return super.action(name, data, options)
    }
    table<T extends TableNames>(name: T, scope?: NameType): Table<RowType<T>> {
        return super.table(name, scope, TableMap[name])
    }
}
